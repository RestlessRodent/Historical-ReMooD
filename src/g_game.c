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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
//      game loop functions, events handling

/* Includes */
#include "doomdef.h"
#include "command.h"
#include "console.h"
#include "dstrings.h"
#include "d_main.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "f_finale.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "i_system.h"
#include "wi_stuff.h"
#include "am_map.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"
#include "r_data.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_sky.h"
#include "s_sound.h"
#include "g_game.h"
#include "g_state.h"
#include "g_input.h"
#include "p_fab.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_argv.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "keys.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_video.h"
#include "p_inter.h"
#include "p_info.h"
#include "p_demcmp.h"
#include "d_rdf.h"
#include "b_bot.h"
#include "st_stuff.h"


// added 8-3-98 increse savegame size from 0x2c000 (180kb) to 512*1024
#define SAVEGAMESIZE    (512*1024)
#define SAVESTRINGSIZE  24

bool_t G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum);

void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);

// demoversion the 'dynamic' version number, this should be == game VERSION
// when playing back demos, 'demoversion' receives the version number of the
// demo. At each change to the game play, demoversion is compared to
// the game version, if it's older, the changes are not done, and the older
// code is used for compatibility.
//
uint8_t demoversion = VERSION;

uint8_t gameepisode;
uint8_t gamemap;
char gamemapname[GAMEMAPNAMESIZE];	// an external wad filename

gamemode_t gamemode = indetermined;	// Game Mode - identify IWAD as shareware, retail etc.
gamemission_t gamemission = doom;
bool_t raven = false;
language_t language = english;	// Language.
bool_t modifiedgame;			// Set if homebrew PWAD stuff has been added.

bool_t paused;

bool_t timingdemo;				// if true, exit with report on completion
bool_t nodrawers;				// for comparative timing purposes
bool_t noblit;					// for comparative timing purposes
tic_t demostarttime;			// for comparative timing purposes

bool_t netgame;					// only true if packets are broadcast
bool_t multiplayer;
bool_t serverside;
bool_t localgame;
bool_t playeringame[MAXPLAYERS];
player_t players[MAXPLAYERS];

int secondarydisplayplayer;		// for splitscreen
int statusbarplayer;			// player who's statusbar is displayed

// (for spying with F12)

tic_t gametic;
tic_t g_ProgramTic = 0;
tic_t levelstarttic;			// gametic at level start
int totalkills, totalitems, totalsecret;	// for intermission

char demoname[32];
bool_t demorecording;
bool_t demoplayback;
uint8_t* demobuffer;
uint8_t* demo_p;
uint8_t* demoend;
bool_t singledemo;				// quit after playing a demo from cmdline

bool_t precache = false;		// if true, load all graphics at start

wbstartstruct_t wminfo;			// parms for world map / intermission

uint8_t* savebuffer;

// changed to 2d array 19990220 by Kin
char player_names[MAXPLAYERS][MAXPLAYERNAME];
char team_names[MAXPLAYERS][MAXPLAYERNAME * 2];

mobj_t* bodyque[BODYQUESIZE];
int bodyqueslot;

void* statcopy;					// for statistics driver

void ShowMessage_OnChange(void)
{
#if 0
	if (!cv_showmessages.value)
		CONL_PrintF("%s\n", MSGOFF);
	else
		CONL_PrintF("%s\n", MSGON);
#endif
}

//  Build an original game map name from episode and map number,
//  based on the game mode (doom1, doom2...)
//
char* G_BuildMapName(int episode, int map)
{
	static char mapname[9];		// internal map name (wad resource name)
	
	if (gamemode == commercial)
		strcpy(mapname, va("MAP%#02d", map));
	else
	{
		mapname[0] = 'E';
		mapname[1] = '0' + episode;
		mapname[2] = 'M';
		mapname[3] = '0' + map;
		mapname[4] = 0;
	}
	return mapname;
}

//
//  Clip the console player mouse aiming to the current view,
//  also returns a signed char for the player ticcmd if needed.
//  Used whenever the player view pitch is changed manually
//
//added:22-02-98:
//changed:3-3-98: do a angle limitation now
short G_ClipAimingPitch(int* aiming)
{
	int limitangle;
	
	//note: the current software mode implementation doesn't have true perspective
	limitangle = 732 << ANGLETOFINESHIFT;
	
	if (*aiming > limitangle)
		*aiming = limitangle;
	else if (*aiming < -limitangle)
		*aiming = -limitangle;
		
	return (*aiming) >> 16;
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
// set secondaryplayer true to build player 2's ticcmd in splitscreen mode
//
int localaiming[MAXSPLITSCREENPLAYERS];
angle_t localangle[MAXSPLITSCREENPLAYERS];

//added:06-02-98: mouseaiming (looking up/down with the mouse or keyboard)
#define KB_LOOKSPEED    (1 << 25)
#define MAXPLMOVE       (forwardmove[1])
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS    (6)

static fixed_t forwardmove[2] = { 25, 50 };
static fixed_t sidemove[2] = { 24, 40 };
static fixed_t angleturn[3] = { 640, 1280, 320 };	// + slow turn

bool_t P_CanUseWeapon(player_t* const a_Player, const weapontype_t a_Weapon);

/* NextWeapon() -- Finds the next weapon in the chain */
// This is for PrevWeapon and NextWeapon
// Rewritten for RMOD Support!
// This uses the fields in weaponinfo_t for ordering info
uint8_t NextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	weaponinfo_t* weapons;
	
	/* Get current weapon info */
	weapons = player->weaponinfo;
	
	/* Get the weapon with the lowest and highest order */
	// Find first gun the player has (so order is correct)
	MostOrder = LeastOrder = 0;
	for (w = 0; w < NUMWEAPONS; w++)
		if (P_CanUseWeapon(player, w))
		{
			// Got the first available gun
			MostOrder = LeastOrder = w;
			break;
		}
	
	// Now go through
	for (w = 0; w < NUMWEAPONS; w++)
	{
		// Can't use this gun?
		if (!P_CanUseWeapon(player, w))
			continue;
		
		// Least
		if (weapons[w].SwitchOrder < weapons[LeastOrder].SwitchOrder)
			LeastOrder = w;
		
		// Most
		if (weapons[w].SwitchOrder > weapons[MostOrder].SwitchOrder)
			MostOrder = w;
	}
	
	/* Look for the current weapon in the weapon list */
	// Well that was easy
	fw = s = g = player->readyweapon;
	
	/* Constantly change the weapon */
	// Prepare variables
	Neg = (step < 0 ? true : false);
	StepsAdd = (Neg ? -1 : 1);
	StepsLeft = step * StepsAdd;
	
	// Go through the weapon list, step times
	while (StepsLeft > 0)
	{
		// Clear variables
		BestDiff = 9999999;		// The worst weapon difference ever
		BestNum = NUMWEAPONS;
		
		// Go through every weapon and find the next in the order
		for (w = 0; w < NUMWEAPONS; w++)
		{
			// Ignore the current weapon (don't want to switch back to it)
			if (w == fw)		// Otherwise BestDiff is zero!
				continue;
			
			// Can't use this gun?
			if (!P_CanUseWeapon(player, w))
				continue;
			
			// Only consider worse/better weapons?
			if ((Neg && weapons[w].SwitchOrder > weapons[fw].SwitchOrder) || (!Neg && weapons[w].SwitchOrder < weapons[fw].SwitchOrder))
				continue;
			
			// Get current diff
			ThisDiff = abs(weapons[fw].SwitchOrder - weapons[w].SwitchOrder);
			
			// Closer weapon?
			if (ThisDiff < BestDiff)
			{
				BestDiff = ThisDiff;
				BestNum = w;
			}
		}
		
		// Found no weapon? Then "loop" around
		if (BestNum == NUMWEAPONS)
		{
			// Switch to the highest gun if going down
			if (Neg)
				fw = MostOrder;
			
			// And if going up, go to the lowest
			else
				fw = LeastOrder;
		}
		
		// Found a weapon
		else
		{
			// Switch to this gun
			fw = BestNum;
		}
		
		// Next step
		StepsLeft--;
	}
	
	/* Return the weapon we want */
	return fw;
}

uint8_t BestWeapon(player_t* player)
{
	return 0;
}

static fixed_t originalforwardmove[2] = { 0x19, 0x32 };
static fixed_t originalsidemove[2] = { 0x18, 0x28 };

//
// G_DoLoadLevel
//
void G_DoLoadLevel(bool_t resetplayer)
{
}

//
// G_Responder
//  Get info needed to make ticcmd_ts for the players.
//
bool_t G_Responder(event_t* ev)
{
	// allow spy mode changes even during the demo
	if (gamestate == GS_LEVEL && ev->type == ev_keydown && ev->data1 == KEY_F12 && (singledemo || !P_XGSVal(PGS_GAMEDEATHMATCH)))
	{
		// spy mode
		do
		{
			g_Splits[0].Display++;
			if (g_Splits[0].Display == MAXPLAYERS)
				g_Splits[0].Display = 0;
		}
		while (!playeringame[g_Splits[0].Display] && g_Splits[0].Display != g_Splits[0].Console);
		
		//added:16-01-98:change statusbar also if playingback demo
		if (singledemo)
			ST_changeDemoView();
			
		//added:11-04-98: tell who's the view
		CONL_PrintF("Viewpoint : %s\n", player_names[g_Splits[0].Display]);
		
		return true;
	}
	// any other key pops up menu if in demos
		// GhostlyDeath <March 17, 2012> -- This messes up the console not opening when pressing any key during boot
#if 0
	if (gameaction == ga_nothing && !singledemo && (demoplayback || gamestate == GS_DEMOSCREEN))
	{
		if (ev->type == ev_keydown)
		{
			M_StartControlPanel();
			return true;
		}
		return false;
	}
#endif
	
	if (gamestate == GS_LEVEL)
	{
		if (!multiplayer)
			if (cht_Responder(ev))
				return true;
		if (ST_Responder(ev))
			return true;		// status window ate it
		if (AM_Responder(ev))
			return true;		// automap ate it
		//added:07-02-98: map the event (key/mouse/joy) to a gamecontrol
	}
	
	if (gamestate == GS_FINALE)
	{
		if (F_Responder(ev))
			return true;		// finale ate the event
	}
	
	switch (ev->type)
	{
		case ev_keydown:
			if (ev->data1 == KEY_PAUSE)
			{
				return true;
			}
			return true;
			
		case ev_keyup:
			return false;		// always let key up events filter down
			
		case ev_mouse:
			return true;		// eat events
			
		case ev_joystick:
			return true;		// eat events
			
		default:
			break;
	}
	
	return false;
}

/* GS_HandleExtraCommands() -- Handles extra commands */
void GS_HandleExtraCommands(ticcmd_t* const a_TicCmd, const int32_t a_PlayerNum)
{
	const void* Rp, *Rb, *Re;
	uint8_t Command;
	
	int i, j, k, l;
	
	D_ProfileEx_t* Profile;
	D_NetClient_t* NC;
	D_NetPlayer_t* NetPlayer;
	player_t* Player;
	
	uint32_t u32[4];
	int32_t i32[4];
	uint16_t u16[4];
	uint8_t u8[4];
	int8_t SplitNum;
	char NameBuf[MAXPLAYERNAME];
	char AltBuf[MAXPLAYERNAME];
	bool_t OK, LegalMove;
	
	B_BotData_t* NewBot;
	
	/* Get pointer base */
	if (a_TicCmd->Type == 1)
	{
		Rb = Rp = a_TicCmd->Ext.DataBuf;
		Re = (uintptr_t)(Rb + a_TicCmd->Ext.DataSize);
	}
	else
	{
		Rb = Rp = a_TicCmd->Std.DataBuf;
		Re = (uintptr_t)(Rb + a_TicCmd->Std.DataSize);
	}
	
	/* Constantly Read Bits */
	do
	{
		Command = ReadUInt8((uint8_t**)&Rp);
		
		// Don't overflow! untrusted!
		if (Command < 0 || Command >= NUMDTCT)
			break;
			
		// Not big enough?
		if ((uintptr_t)(Rp + c_TCDataSize[Command]) > (uintptr_t)(Re))
			break;
			
		// Clear
		memset(NameBuf, 0, sizeof(NameBuf));
		
		// Which command?
		switch (Command)
		{
				// Player Joins
			case DTCT_JOIN:
				// Read Data
				u32[0] = LittleReadUInt32((uint32_t**)&Rp);
				u16[0] = LittleReadUInt16((uint16_t**)&Rp);
				u32[1] = LittleReadUInt32((uint32_t**)&Rp);
				u32[2] = LittleReadUInt32((uint32_t**)&Rp);	// Profile
				u32[3] = LittleReadUInt32((uint32_t**)&Rp);	// Screen Instance
				u8[0] = ReadUInt8((uint8_t**)&Rp);
				
				for (i = 0; i < MAXPLAYERNAME; i++)
					NameBuf[i] = ReadUInt8((uint8_t**)&Rp);
				NameBuf[MAXPLAYERNAME - 1] = 0;
				
				for (i = 0; i < MAXPLAYERNAME; i++)
					AltBuf[i] = ReadUInt8((uint8_t**)&Rp);
				AltBuf[MAXPLAYERNAME - 1] = 0;
				
				// Determine spot for player (this is in case 1000 players
				// join in a single tic).
				while (u16[0] < MAXPLAYERS)
					if (playeringame[u16[0]])
						u16[0]++;
					else
						break;
				
				// Fits in the game
				if (u16[0] < MAXPLAYERS)
				{
					// Find player host (if in net game)
					NC = NULL;
					if (!demoplayback)
						NC = D_NCFindClientByID(u32[0]);
					
					// Clear
					Profile = NULL;
					NetPlayer = NULL;
					NewBot = NULL;
					SplitNum = 0;
					
					// Playing Demo (no netclients, but show on screen)
					OK = false;
					if (demoplayback)
					{
						// Fits in screen and matches demo host
						if (G_GetDemoHostID() == 0 ||
							u32[0] == G_GetDemoHostID())
							if (g_SplitScreen < 3)
							{
								SplitNum = ++g_SplitScreen;
								OK = true;
							}
					}
					
					// Give arbitration to player
					else if (NC)
					{
						// Add to arbs list
						for (i = 0; i < NC->NumArbs; i++)
							if (!NC->Arbs[i])
								break;
						
						// No room?
						if (i >= NC->NumArbs)
						{
							Z_ResizeArray((void**)&NC->Arbs, sizeof(*NC->Arbs),
								NC->NumArbs, NC->NumArbs + 1);
							NC->NumArbs++;
						}
						
						// Allocate net player here
						NC->Arbs[i] = NetPlayer = D_NCSAllocNetPlayer();
						NetPlayer->NetClient = NC;
						NetPlayer->ProcessID = u32[3];
						
						if (NC->IsLocal)
						{
							// Always legal
							LegalMove = true;
							
							// Bot?
							if (NC->IsServer && (u32[1] & DTCJF_ISBOT))
							{
								// Set as bot
								NetPlayer->Type = DNPT_BOT;
								
								// Don't mess with bot splits
								LegalMove = false;
							}
							
							// Standard Player
							else
							{
								NetPlayer->Type = DNPT_LOCAL;
								
								// Locate split based on instance
								SplitNum = D_NCSFindSplitByProcess(u32[3]);
								
								// Illegal Split?
								if (SplitNum < 0 && g_SplitScreen < 3)
								{
									// See if there is room in a split
									for (SplitNum = 0;
										SplitNum <= (g_SplitScreen + 1); SplitNum++)
										if (!D_ScrSplitHasPlayer(SplitNum))
										{
											LegalMove = true;
											break;
										}
									
									// It fits
									if (SplitNum == (g_SplitScreen + 1) &&
										g_SplitScreen < 3)
									{
										LegalMove = true;
										SplitNum = ++g_SplitScreen;
									}
								}
								
								// Legal
								else
									LegalMove = true;
							
								// Obtain profile (if possible)
								Profile = D_FindProfileExByInstance(u32[2]);
								NetPlayer->Profile = Profile;
							}
						}
						else
							NetPlayer->Type = DNPT_NETWORK;
					}
					
					// Fill player spot
					playeringame[u16[0]] = true;
					
					// Initialize Player
					Player = G_AddPlayer(u16[0]);
					D_NetSetPlayerName(u16[0], NameBuf);
					Player->skincolor = u8[0];
					// TODO FIXME: Hexen Class (AltBuf)
					
					if (u32[1] & DTCJF_MONSTERTEAM)
						Player->CounterOpPlayer = true;
					
					// Link player to profile
					Player->NetPlayer = NetPlayer;
					Player->ProfileEx = Profile;
					
					// Link netplayer
					if (NetPlayer)
					{
						NetPlayer->Player = Player;
						NetPlayer->Profile = Profile;
					}
					
					// Setup Bot Player
					if (NetPlayer->Type == DNPT_BOT)
					{
						// Initialize Bot
						NewBot = NetPlayer->BotData = B_InitBot(NetPlayer, B_GHOST_TemplateByID(u32[2]));
						g_GotBots = true;
					}
					
					// Send server local profile info
					D_NCReqPrefChange(Profile, NewBot, u16[0]);
					
					// Finish off split screen
					if ((NC && NC->IsLocal && LegalMove) || (demoplayback && OK))
					{
						g_Splits[SplitNum].Waiting = false;
						g_Splits[SplitNum].Active = true;
						g_Splits[SplitNum].Display = 
							g_Splits[SplitNum].Console = u16[0];
						g_Splits[SplitNum].Profile = Profile;
						
						R_ExecuteSetViewSize();
					}
				}
				
				// Recount player total
				for (j = 0, i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i])
						j++;
				
				// If more than 1 player, set multiplayer modes
				if (j > 1)
				{
					P_XGSSetValue(true, PGS_GAMESPAWNMULTIPLAYER, 1);
					P_XGSSetValue(true, PGS_COMULTIPLAYER, 1);
				}
				
				// Debug
				if (g_NetDev)
					CONL_PrintF("NET: Join (%x, %u, %x, %x, %u, %s, %s)\n",
							u32[0], u16[0], u32[1], u32[2], u8[0], NameBuf, AltBuf
						);
				break;
				
				// Map Changes
			case DTCT_MAPCHANGE:
				// Read Data
				u8[0] = ReadUInt8((uint8_t**)&Rp);
				
				for (i = 0; i < 8; i++)
					NameBuf[i] = ReadUInt8((uint8_t**)&Rp);
				
				// Change the level
				if (!P_ExLoadLevel(P_FindLevelByNameEx(NameBuf, NULL), 0))
					CONL_PrintF("Level \"%s\" failed to load.\n", NameBuf);
					
				// Debug
				if (g_NetDev)
					CONL_PrintF("NET: Map (%x, %s)\n",
							u8[0], NameBuf
						);
				break;
				
				// Variable Change
			case DTCT_GAMEVAR:
				// Read Data
				u32[0] = LittleReadUInt32((uint32_t**)&Rp);
				i32[0] = LittleReadInt32((int32_t**)&Rp);
				
				// Change Variable
				P_XGSSetValue(true, u32[0], i32[0]);
				break;
			
			default:
				Command = 0;
				break;
		}
	} while (Command != 0);
}

extern tic_t g_WatchTic;

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(void)
{
	uint32_t i;
	int buf;
	ticcmd_t* cmd;
	tic_t ThisTime;
	ticcmd_t GlobalCmd;
	
	/* Reduce Menu */
	if (g_ResumeMenu > 0)
		g_ResumeMenu--;
	
	//for (i = 0; i == 0 && (gametic == g_WatchTic);)
		;
	
	// GhostlyDeath <May 13, 2012> -- Run Commands
	D_NCRunCommands();
	
	// do player reborns if needed
	if (gamestate == GS_LEVEL)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i])
			{
				if (players[i].playerstate == PST_REBORN)
				{
					// Player is on monster's side
					if (players[i].CounterOpPlayer)
						P_ControlNewMonster(&players[i]);
						
					// Otherwise make player
					else
						G_DoReborn(i);
				}
				
				if (players[i].st_inventoryTics)
					players[i].st_inventoryTics--;
			}
		}
	}
	
	// do things to change the game state
	while (gameaction != ga_nothing)
		switch (gameaction)
		{
			case ga_completed:
				G_DoCompleted();
				break;
				
			case ga_worlddone:
				G_DoWorldDone();
				break;
				
			case ga_nothing:
				break;
				
			default:
				I_Error("gameaction = %d\n", gameaction);
		}
		
	buf = gametic % BACKUPTICS;
	//buf = D_SyncNetMapTime() % BACKUPTICS;
	
	// read/write demo and check turbo cheat
	ThisTime = D_SyncNetMapTime();
	
	/* Load network tic */
	D_LoadNetTic();
	
	/* Global Commands */
	// Clear
	memset(&GlobalCmd, 0, sizeof(GlobalCmd));
	
	// Read Global Tic Commands
	if (demoplayback)
		G_ReadDemoGlobalTicCmd(&GlobalCmd);
	else
		D_NetReadGlobalTicCmd(&GlobalCmd);
	
	// Write Global Tic Commands
	if (demorecording)
		G_WriteDemoGlobalTicCmd(&GlobalCmd);
	D_NetWriteGlobalTicCmd(&GlobalCmd);
	
	/* Player Commands */
	// Read Individual Player Tic Commands
	for (i = 0; i < MAXPLAYERS; i++)
		// BP: i==0 for playback of demos 1.29 now new players is added with xcmd
		if ((playeringame[i] || i == 0) && !dedicated)
		{
			memset(&players[i].cmd, 0, sizeof(players[i].cmd));
			cmd = &players[i].cmd;
			
			// Read Command
			if (demoplayback)
				G_ReadDemoTiccmd(cmd, i);
			else
				D_NetReadTicCmd(cmd, i);
			
			// Write Command
			if (demorecording)
				G_WriteDemoTiccmd(cmd, i);
			D_NetWriteTicCmd(cmd, i);
		}
	
	/* Transmit Network Commands */
	D_NetXMitCmds();
	
	/* Handle Commands */
	// Process Global Commands
	if (GlobalCmd.Type >= 1)
		GS_HandleExtraCommands(&GlobalCmd, -1);
		
	// Per Player Commands
	for (i = 0; i < MAXPLAYERS; i++)
		if ((playeringame[i] || i == 0) && !dedicated)
			GS_HandleExtraCommands(cmd, i);
	
	// Set new time
	g_DemoTime = ThisTime;
	
	// do main actions
	switch (gamestate)
	{
		case GS_LEVEL:
			B_GHOST_Ticker();
			P_Ticker();			// tic the game
			ST_Ticker();
			AM_Ticker();
			ST_TickerEx();
			break;
			
		case GS_INTERMISSION:
			WI_Ticker();
			break;
			
		case GS_FINALE:
			F_Ticker();
			break;
		
			// Waiting for join window
		case GS_WAITFORJOINWINDOW:
			break;
			
		case GS_WAITINGPLAYERS:
		case GS_DEDICATEDSERVER:
		case GS_NULL:
		default:
			// do nothing
			break;
	}
	
	/* Check Demo */
	G_CheckDemoStatus();
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerFinishLevel
//  Can when a player completes a level.
//
void G_PlayerFinishLevel(int player)
{
	player_t* p;
	int i;
	
	p = &players[player];
	memset(p->powers, 0, sizeof(p->powers));
	for (i = 0; i < p->inventorySlotNum; i++)
		if (p->inventory[i].count > 1)
			p->inventory[i].count = 1;
	//if (gamemode == heretic)
	p->weaponinfo = wpnlev1info;	// cancel power weapons
	//else
	//  p->weaponinfo = doomweaponinfo;
	p->cards = 0;
	p->mo->flags &= ~MF_SHADOW;	// cancel invisibility
	p->extralight = 0;			// cancel gun flashes
	p->fixedcolormap = 0;		// cancel ir gogles
	p->damagecount = 0;			// no palette changes
	p->bonuscount = 0;
	
	if (p->chickenTics)
	{
		p->readyweapon = p->mo->special1;	// Restore weapon
		p->chickenTics = 0;
	}
	p->rain1 = NULL;
	p->rain2 = NULL;
}

// added 2-2-98 for hacking with dehacked patch
int initial_health = 100;		//MAXHEALTH;
int initial_bullets = 50;

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//
void G_PlayerReborn(int player)
{
	player_t* p;
	int i;
	uint16_t frags[MAXPLAYERS];
	int killcount;
	int itemcount;
	int secretcount;
	uint16_t addfrags;
	bool_t* weaponowned;
	weapontype_t* FavoriteWeapons;
	int* ammo;
	int* maxammo;
	uint32_t FraggerID;
	
	D_ProfileEx_t* PEp;
	D_NetPlayer_t* NPp;
	
	//from Boris
	int skincolor;
	//char favoritweapon[NUMWEAPONS];
	bool_t originalweaponswitch;
	bool_t autoaim;
	int skin;					//Fab: keep same skin
	int32_t TotalFrags, TotalDeaths;
	bool_t Given;
	
	PEp = players[player].ProfileEx;
	NPp = players[player].NetPlayer;
	
	memcpy(frags, players[player].frags, sizeof(frags));
	addfrags = players[player].addfrags;
	killcount = players[player].killcount;
	itemcount = players[player].itemcount;
	secretcount = players[player].secretcount;
	TotalFrags = players[player].TotalFrags;
	TotalDeaths = players[player].TotalDeaths;
	FraggerID = players[player].FraggerID;
	
	FavoriteWeapons = players[player].FavoriteWeapons;
	weaponowned = players[player].weaponowned;
	ammo = players[player].ammo;
	maxammo = players[player].maxammo;
	
	//from Boris
	skincolor = players[player].skincolor;
	originalweaponswitch = players[player].originalweaponswitch;
	//memcpy(favoritweapon, players[player].favoritweapon, NUMWEAPONS);
	autoaim = players[player].autoaim_toggle;
	skin = players[player].skin;
	
	p = &players[player];
	memset(p, 0, sizeof(*p));
	
	memcpy(players[player].frags, frags, sizeof(players[player].frags));
	players[player].addfrags = addfrags;
	players[player].killcount = killcount;
	players[player].itemcount = itemcount;
	players[player].secretcount = secretcount;
	players[player].TotalFrags = TotalFrags;
	players[player].TotalDeaths = TotalDeaths;
	players[player].FraggerID = FraggerID;
	players[player].FavoriteWeapons = FavoriteWeapons;
	
	// Weapons
	if (!weaponowned)
		weaponowned = Z_Malloc(sizeof(*weaponowned) * NUMWEAPONS, PU_STATIC, NULL);
	memset(weaponowned, 0, sizeof(*weaponowned) * NUMWEAPONS);
	players[player].weaponowned = weaponowned;

	// Ammo
	if (!ammo)
		ammo = Z_Malloc(sizeof(*ammo) * NUMAMMO, PU_STATIC, NULL);
	memset(ammo, 0, sizeof(*ammo) * NUMAMMO);
	players[player].ammo = ammo;
	
	// Max Ammo
	if (!maxammo)
		maxammo = Z_Malloc(sizeof(*maxammo) * NUMAMMO, PU_STATIC, NULL);
	memset(maxammo, 0, sizeof(*maxammo) * NUMAMMO);
	players[player].maxammo = maxammo;
	
	// save player config truth reborn
	players[player].skincolor = skincolor;
	players[player].originalweaponswitch = originalweaponswitch;
	players[player].autoaim_toggle = autoaim;
	players[player].skin = skin;
	
	p->usedown = p->attackdown = true;	// don't do anything immediately
	p->playerstate = PST_LIVE;
	p->health = initial_health;
	
	p->weaponinfo = wpnlev1info;
	
	// GhostlyDeath <April 13, 2012> -- Give player starting weapons
	p->readyweapon = p->pendingweapon = NUMWEAPONS;
	for (i = 0; i < NUMWEAPONS; i++)
		// Only see if the weapon is unlocked
		if (P_WeaponIsUnlocked(i))
			// Is this a starting weapon
			if (p->weaponinfo[i]->WeaponFlags & WF_STARTINGWEAPON)
			{
				// Set as owned
				p->weaponowned[i] = true;
				
				// Choose this gun?
				if ((p->readyweapon == NUMWEAPONS) ||
					(p->readyweapon != NUMWEAPONS && p->weaponinfo[i]->NoAmmoOrder > p->weaponinfo[p->readyweapon]->NoAmmoOrder))
					p->readyweapon = p->pendingweapon = i;
			}
	
	// GhostlyDeath <April 13, 2012> -- Give player starting ammo
	for (i = 0; i < NUMAMMO; i++)
		p->ammo[i] = ammoinfo[i]->StartingAmmo;
	
	players[player].ProfileEx = PEp;
	players[player].NetPlayer = NPp;
	
	for (i = 0; i < NUMAMMO; i++)
		p->maxammo[i] = ammoinfo[i]->MaxAmmo;
	
	// GhostlyDeath <April 26, 2012> -- Health and Armor
	p->MaxHealth[0] = p->MaxArmor[0] = 100;
	p->MaxHealth[1] = p->MaxArmor[1] = 200;
	
	// GhostlyDeath <June 6, 2012> -- Stat Mods
	if (P_XGSVal(PGS_PLSPAWNWITHMAXSTATS))
	{
		p->health = p->MaxHealth[1];
		p->armorpoints = p->MaxArmor[1];
		p->armortype = 2;
	}
	
	// GhostlyDeath <June 6, 2012> -- Weapon Mods
	for (i = 0; i < NUMWEAPONS; i++)
	{
		// Clear
		Given = false;
		
		// Locked weapon?
		if (!P_WeaponIsUnlocked(i))
			continue;
		
		// Normal Gun?
		if (P_XGSVal(PGS_PLSPAWNWITHMAXGUNS))
			if (!(p->weaponinfo[i]->WeaponFlags & WF_SUPERWEAPON))
				p->weaponowned[i] |= Given |= true;
				
		// Super Gun?
		if (P_XGSVal(PGS_PLSPAWNWITHSUPERGUNS))
			if ((p->weaponinfo[i]->WeaponFlags & WF_SUPERWEAPON))
				p->weaponowned[i] |= Given |= true;
		
		// Gave something? Then give max ammo
		if (Given)
			if (p->weaponinfo[i]->ammo >= 0 && p->weaponinfo[i]->ammo < NUMAMMO)
				p->ammo[p->weaponinfo[i]->ammo] = p->maxammo[p->weaponinfo[i]->ammo];
	}
	
	/* Switch to favorite weapon? */
	// Before 1.0a, favorite weapons on spawn is not a happening thing
	if (!p->originalweaponswitch)
		P_PlayerSwitchToFavorite(p, true);
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
bool_t G_CheckSpot(int playernum, mapthing_t* mthing, const bool_t a_NoFirstMo)
{
	fixed_t x;
	fixed_t y;
	subsector_t* ss;
	angle_t an;
	mobj_t* mo;
	int i;
	
	// added 25-4-98 : maybe there is no player start
	if (!mthing || mthing->type < 0)
		return false;
	
	if (!a_NoFirstMo)
		if (!players[playernum].mo)
		{
			// first spawn of level, before corpses
			for (i = 0; i < playernum; i++)
				// added 15-1-98 check if player is in game (mistake from id)
				if (playeringame[i] && (players[i].mo && players[i].mo->x == mthing->x << FRACBITS && players[i].mo->y == mthing->y << FRACBITS))
					return false;
			return true;
		}
	
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	ss = R_PointInSubsector(x, y);
	
	// check for respawn in team-sector
	if (!P_XGSVal(PGS_CODISABLETEAMPLAY))
		if (ss->sector->teamstartsec)
		{
			if (P_XGSVal(PGS_GAMETEAMPLAY) == 1)
			{
				// color
				if (players[playernum].skincolor != (ss->sector->teamstartsec - 1))	// -1 because wanted to know when it is set
					return false;
			}
			else if (P_XGSVal(PGS_GAMETEAMPLAY) == 2)
			{
				// skins
				if (players[playernum].skin != (ss->sector->teamstartsec - 1))	// -1 because wanted to know when it is set
					return false;
			}
		}
	
	// GhostlyDeath <April 21, 2012> -- Check 
	if (a_NoFirstMo && P_XGSVal(PGS_CORADIALSPAWNCHECK))
	{
		if (!P_CheckPosRadius(x, y, 20 << FRACBITS))
			return false;
	}
	
	// Otherwise compare against object
	else
	{
		if (!P_CheckPosition(players[playernum].mo, x, y, (P_XGSVal(PGS_COLESSSPAWNSTICKING) ? PCPF_FORSPOTCHECK : 0)))
			return false;
	}
		
	// flush an old corpse if needed
		// GhostlyDeath <April 20, 2012> -- This is quite useless here especially when there are 32 players!
	if (!P_XGSVal(PGS_COBETTERPLCORPSEREMOVAL))
		if (!a_NoFirstMo)
		{
			if (bodyqueslot >= BODYQUESIZE)
				P_RemoveMobj(bodyque[bodyqueslot % BODYQUESIZE]);
			bodyque[bodyqueslot % BODYQUESIZE] = players[playernum].mo;
			bodyqueslot++;
		}
	
	// spawn a teleport fog
	if (players[playernum].mo)
	{
		// TODO: Vanilla comp: an = (ANG45 * (mthing->angle / 45)) >> ANGLETOFINESHIFT;
		an = (ANG45 * (mthing->angle / 45));
		an >>= ANGLETOFINESHIFT;
	
		mo = P_SpawnMobj(x + 20 * finecosine[an], y + 20 * finesine[an], ss->sector->floorheight, INFO_GetTypeByName("TeleportFog"));
	
		//added:16-01-98:consoleplayer -> displayplayer (hear snds from viewpt)
		// removed 9-12-98: why not ????
		if (players[g_Splits[0].Display].viewz != 1)
			S_StartSound(mo, sfx_telept);	// don't start sound on first frame
	}
		
	return true;
}

/* GS_ClusterTraverser() -- Cluster Traverser */
static bool_t GS_ClusterTraverser(intercept_t* in, void* const a_Data)
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

/* G_ClusterSpawnPlayer() -- Spawns player in cluster spot */
bool_t G_ClusterSpawnPlayer(const int PlayerID, const bool_t a_CheckOp)
{
	mapthing_t** Spots;
	size_t NumSpots, i, s;
	int x, y, bx, by;
	mapthing_t OrigThing, FakeThing;
	subsector_t* SubS;
	bool_t RandomSpot;
	bool_t* Tried;
	bool_t PreDiamond;
	
	static const uint8_t SpawnDiamond[9][9] =
	{
		{0, 0, 0, 0, 1, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 1, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 0, 1, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 0, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 1, 0, 0, 0, 0},
	};
	
	/* Bad player id? */
	if (PlayerID < 0 || PlayerID >= MAXPLAYERS)
		return false;
	
	/* Which spots to prefer? */
	// Deathmatch
	if (P_XGSVal(PGS_GAMEDEATHMATCH) || (!P_XGSVal(PGS_GAMEDEATHMATCH) && a_CheckOp))
	{
		PreDiamond = false;
		RandomSpot = true;
		Spots = deathmatchstarts;
		NumSpots = numdmstarts;
		Tried = Z_Malloc(sizeof(*Tried) * NumSpots, PU_STATIC, NULL);
	}
	
	// Coop
	else if (!P_XGSVal(PGS_GAMEDEATHMATCH) || (P_XGSVal(PGS_GAMEDEATHMATCH) && a_CheckOp))
	{
		PreDiamond = true;
		RandomSpot = false;
		Spots = playerstarts;
		NumSpots = MAXPLAYERS;
		Tried = NULL;
	}
	
	/* Determine offset base */
	if (a_CheckOp || P_XGSVal(PGS_GAMEDEATHMATCH))
		bx = by = 2;
	else
		bx = by = 4;
	
	/* Go through each spot */
	for (s = 0; s < NumSpots; s++)
	{
		// Randomize?
		if (RandomSpot)
		{
			// Determine random spot
			do
			{
				i = P_Random() % NumSpots;
			} while (Tried[i]);
			
			// Mark as tried
			Tried[i] = true;
		}
		
		// Normal through the list
		else
			i = s;
		
		// No actual spot here?
		if (!Spots[i])
			continue;
		
		// Copy original thing
		OrigThing = *Spots[i];
		
		// Try spawning in different spots
		for (x = -bx; x <= bx; x++)
			for (y = -by; y <= by; y++)
			{
				// Coop -- Spawn in pre-made diamond
				if (!P_XGSVal(PGS_GAMEDEATHMATCH))
				{
					if (!SpawnDiamond[x + bx][y + by])
						continue;
				}
				
				// DM -- Spawn in plus pattern only
				else
				{
					if (x == y || (x == 0 && y == 0))
						continue;
				}
				
				// Create fake thing
				FakeThing = OrigThing;
				FakeThing.type = PlayerID + 1;
				
				// Move thing location
				FakeThing.x += (x * 36);
				FakeThing.y += (y * 36);
				
				// See if a straight line can be drawn between these two spots
				if (!P_CheckSightLine(
							((fixed_t)OrigThing.x) << FRACBITS,
							((fixed_t)OrigThing.y) << FRACBITS,
							((fixed_t)FakeThing.x) << FRACBITS,
							((fixed_t)FakeThing.y) << FRACBITS
						))
					continue;
				
				// Draw another line from the soure to destination
				if (!P_PathTraverse(
							((fixed_t)OrigThing.x) << FRACBITS,
							((fixed_t)OrigThing.y) << FRACBITS,
							((fixed_t)FakeThing.x) << FRACBITS,
							((fixed_t)FakeThing.y) << FRACBITS,
							PT_ADDLINES,
							GS_ClusterTraverser,
							NULL
						))
					continue;
				
				// Draw line from thing corner (corss section BL to TR)
				if (!P_PathTraverse(
							((fixed_t)FakeThing.x - 16) << FRACBITS,
							((fixed_t)FakeThing.y - 16) << FRACBITS,
							((fixed_t)FakeThing.x + 16) << FRACBITS,
							((fixed_t)FakeThing.y + 16) << FRACBITS,
							PT_ADDLINES | PT_ADDTHINGS,
							GS_ClusterTraverser,
							NULL
						))
					continue;
				
				// Draw line from thing corner (corss section TL to BR)
				if (!P_PathTraverse(
							((fixed_t)FakeThing.x - 16) << FRACBITS,
							((fixed_t)FakeThing.y + 16) << FRACBITS,
							((fixed_t)FakeThing.x + 16) << FRACBITS,
							((fixed_t)FakeThing.y - 16) << FRACBITS,
							PT_ADDLINES | PT_ADDTHINGS,
							GS_ClusterTraverser,
							NULL
						))
					continue;
				
				// Obtain subsector to this point
				SubS = R_PointInSubsector(((fixed_t)FakeThing.x) << FRACBITS, ((fixed_t)FakeThing.y) << FRACBITS);
				
				// No subsector here
				if (!SubS)
					continue;
				
				// Check to see if the player can actually fit
				if ((SubS->sector->ceilingheight - SubS->sector->floorheight) < (56 << FRACBITS))
					continue;
				
				// Check to see if something can be spawned here
				if (!G_CheckSpot(PlayerID, &FakeThing, true))
					continue;
				
				// Spawn player here
				P_SpawnPlayer(&FakeThing);
				
				// Clear spawn spot (since it does not actually exist)
				if (players[PlayerID].mo)
					players[PlayerID].mo->spawnpoint = NULL;
				
				// Clear tried before we leave
				if (Tried)
					Z_Free(Tried);
				
				// Success here!
				return true;
			}
	}
	
	/* Clear tried */
	if (Tried)
		Z_Free(Tried);
	
	/* Did not find a spot nor spawned a player */
	return false;
}

//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
bool_t G_DeathMatchSpawnPlayer(int playernum)
{
	int i, j, n;
	
	if (!numdmstarts)
	{
		CONL_PrintF("No deathmatch start in this map, falling back to Coop starts!");
		P_SpawnPlayer(playerstarts[playernum]);
		return false;
	}
	
	if (P_XGSVal(PGS_COONLYTWENTYDMSPOTS))
		n = 20;
	else
		n = 64;
	
	// GhostlyDeath <April 21, 2012> -- Spawn clustering (extra invisible spots)
	if (P_XGSVal(PGS_PLSPAWNCLUSTERING))
	{
		// Try DM starts first
		if (G_ClusterSpawnPlayer(playernum, false))
			return true;
			
		// Then try Coop Starts
		if (G_ClusterSpawnPlayer(playernum, true))
			return true;
	}
	
	for (j = 0; j < n; j++)
	{
		i = P_Random() % numdmstarts;
		
		if (G_CheckSpot(playernum, deathmatchstarts[i], false))
		{
			deathmatchstarts[i]->type = playernum + 1;
			P_SpawnPlayer(deathmatchstarts[i]);
			return true;
		}
	}

	// no good spot, so the player will probably get stuck
	if (P_XGSVal(PGS_COALLOWSTUCKSPAWNS))
	{
		P_SpawnPlayer(playerstarts[playernum]);
		return true;
	}
	
	return false;
}

void G_CoopSpawnPlayer(int playernum)
{
	int i;
	
	// no deathmatch use the spot
	if (G_CheckSpot(playernum, playerstarts[playernum], false))
	{
		P_SpawnPlayer(playerstarts[playernum]);
		return;
	}
	// try to spawn at one of the other players spots
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (G_CheckSpot(playernum, playerstarts[i], false))
		{
			playerstarts[i]->type = playernum + 1;	// fake as other player
			P_SpawnPlayer(playerstarts[i]);
			playerstarts[i]->type = i + 1;	// restore
			return;
		}
		// he's going to be inside something.  Too bad.
	}
	
	// GhostlyDeath <April 21, 2012> -- Spawn clustering (extra invisible spots)
	if (P_XGSVal(PGS_PLSPAWNCLUSTERING))
	{
		// Try Coop starts first
		if (G_ClusterSpawnPlayer(playernum, false))
			return;
			
		// Then try DM Starts
		if (G_ClusterSpawnPlayer(playernum, true))
			return;
	}
	
	if (P_XGSVal(PGS_COALLOWSTUCKSPAWNS) || (!P_XGSVal(PGS_COALLOWSTUCKSPAWNS) && localgame))
		P_SpawnPlayer(playerstarts[playernum]);
	else
	{
		int selections;
		
		if (!numdmstarts)
			I_Error("No deathmatch start in this map !");
		selections = P_Random() % numdmstarts;
		deathmatchstarts[selections]->type = playernum + 1;
		P_SpawnPlayer(deathmatchstarts[selections]);
	}
}

//
// G_DoReborn
//
void G_DoReborn(int playernum)
{
	player_t* player = &players[playernum];
	
	/* Single Player Mode */
	if (!P_XGSVal(PGS_COMULTIPLAYER))
		P_ExLoadLevel(g_CurrentLevelInfo, 0);

	/* Multiplayer Mode */
	else
	{
		// respawn at the start
		
		// first dissasociate the corpse
		if (player->mo)
		{
			player->mo->player = NULL;
			player->mo->flags2 &= ~MF2_DONTDRAW;
		}
		// spawn at random spot if in death match
		if (P_XGSVal(PGS_GAMEDEATHMATCH))
		{
			if (G_DeathMatchSpawnPlayer(playernum))
				return;
		}
		
		G_CoopSpawnPlayer(playernum);
	}
}

/* G_AddPlayer() -- Adds player to game */
player_t* G_AddPlayer(int playernum)
{
	int i;
	player_t* p = &players[playernum];
	playeringame[playernum] = true;
	
	/* Just in case */
	if (p->FavoriteWeapons)
		Z_Free(p->FavoriteWeapons);
	if (p->weaponowned)
		Z_Free(p->weaponowned);
	if (p->ammo)
		Z_Free(p->ammo);
	if (p->maxammo)
		Z_Free(p->maxammo);
	
	/* Wipe */
	memset(p, 0, sizeof(*p));
	
	/* Initialize */
	p->playerstate = PST_REBORN;
	p->weaponinfo = wpnlev1info;
	p->autoaim_toggle = true;
	
	/* Build Fragger ID */
	p->FraggerID = 0;
	for (i = 0; i < 31; i++)
	{
		p->FraggerID <<= 1;
		p->FraggerID ^= M_Random();
	}
	
	/* Initialize */
	G_InitPlayer(p);
	
	/* Return it */
	return &players[playernum];
}

/* G_InitPlayer() -- Initializes Player */
void G_InitPlayer(player_t* const a_Player)
{
	int i, j, k, l;
	int32_t pNum;
	
	/* Check */
	if (!a_Player)
		return;
	
	/* Get Player Number */
	pNum = a_Player - players;
	
	/* Allocate */
	// Guns
	a_Player->FavoriteWeapons = Z_Malloc(sizeof(*a_Player->FavoriteWeapons) * (NUMWEAPONS + 2), PU_STATIC, NULL);
	a_Player->weaponowned = Z_Malloc(sizeof(*a_Player->weaponowned) * NUMWEAPONS, PU_STATIC, NULL);
	
	// Ammo
	a_Player->maxammo = Z_Malloc(sizeof(*a_Player->maxammo) * NUMAMMO, PU_STATIC, NULL);
	a_Player->ammo = Z_Malloc(sizeof(*a_Player->ammo) * NUMAMMO, PU_STATIC, NULL);
	
	/* Clear Totals */
	a_Player->addfrags = 0;
	a_Player->killcount = a_Player->itemcount = a_Player->secretcount = 0;
	a_Player->TotalFrags = a_Player->TotalDeaths = 0;
	
	for (i = 0; i < MAXPLAYERS; i++)
		a_Player->frags[i] = 0;
	
	/* Find if the player is on the screen */
	
	/* Match angle */
	
	/* Update Camera */
	// Use default settings
	a_Player->CamDist = 128 << FRACBITS;
	a_Player->CamHeight = 20 << FRACBITS;
	a_Player->CamSpeed = 16384;
	
	if (a_Player->camera.chase)
		P_ResetCamera(a_Player);
	
	/* Default Name of Player */
	memset(player_names[pNum], 0, sizeof(*player_names[pNum] * MAXPLAYERNAME));
	snprintf(player_names[pNum], MAXPLAYERNAME - 1, "Player %i", (pNum + 1));
	
	/* Default Weapon Order */
#if 0
	// Directly mapped weapon IDs
	for (i = 0; i < NUMWEAPONS; i++)
		a_Player->FavoriteWeapons[i] = i;
#else
	for (k = j = i = 0; i < NUMWEAPONS; i++)
	{
		// Find spot to place this weapons
		for (j = 0; j < k; j++)
			if (wpnlev1info[i]->SwitchOrder < wpnlev1info[a_Player->FavoriteWeapons[j]]->SwitchOrder)
				break;
		
		// At end?
		if (j == k || k == 0)
			a_Player->FavoriteWeapons[k++] = i;
		
		// Move everything over
		else
		{
			for (l = k; l > j; l--)
				a_Player->FavoriteWeapons[l] = a_Player->FavoriteWeapons[l - 1];
			a_Player->FavoriteWeapons[j] = i;
			k++;
		}
	}
#endif
}

//
// G_DoCompleted
//
bool_t secretexit;

void G_ExitLevel(void)
{
	if (gamestate == GS_LEVEL)
	{
		secretexit = false;
		gameaction = ga_completed;
	}
}

// Here's for the german edition.
void G_SecretExitLevel(void)
{
	if (gamestate == GS_LEVEL)
	{
		secretexit = true;
		gameaction = ga_completed;
	}
}

void G_DoCompleted(void)
{
	int i;
	
	gameaction = ga_nothing;
	
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			G_PlayerFinishLevel(i);	// take away cards and stuff
			
	if (automapactive)
		AM_Stop();
	
	// GhostlyDeath <May 5, 2012> -- Secret Exit?
	if (secretexit)
		wminfo.next = 1;
	else
		wminfo.next = 0;
	
	// Did secret level?
	for (i = 0; i < MAXPLAYERS; i++)
		players[i].didsecret = true;
	
	if (!dedicated)
		wminfo.didsecret = players[g_Splits[0].Console].didsecret;
	wminfo.epsd = gameepisode - 1;
	wminfo.last = gamemap - 1;
	wminfo.maxkills = totalkills;
	wminfo.maxitems = totalitems;
	wminfo.maxsecret = totalsecret;
	wminfo.maxfrags = 0;
	wminfo.partime = TICRATE * g_CurrentLevelInfo->ParTime;
	wminfo.pnum = g_Splits[0].Console;
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		wminfo.plyr[i].in = playeringame[i];
		wminfo.plyr[i].skills = players[i].killcount;
		wminfo.plyr[i].sitems = players[i].itemcount;
		wminfo.plyr[i].ssecret = players[i].secretcount;
		wminfo.plyr[i].stime = leveltime;
		memcpy(wminfo.plyr[i].frags, players[i].frags, sizeof(wminfo.plyr[i].frags));
		wminfo.plyr[i].addfrags = players[i].addfrags;
	}
	
	gamestate = GS_INTERMISSION;
	automapactive = false;
	automapoverlay = false;
	
	if (statcopy)
		memcpy(statcopy, &wminfo, sizeof(wminfo));
		
	WI_Start(&wminfo);
}

//
// G_NextLevel (WorldDone)
//
// init next level or go to the final scene
// called by end of intermision screen (wi_stuff)
void G_NextLevel(void)
{
	gameaction = ga_worlddone;
	
	if (secretexit)
		players[g_Splits[0].Console].didsecret = true;
}

void G_DoWorldDone(void)
{
	P_LevelInfoEx_t* NewInfo;

#if 0
	if (P_XGSVal(PGS_COLINEARMAPTRAVERSE))
	{
#endif
		// Clear Info
		NewInfo = NULL;
		
		// Secret Exit?
		if (wminfo.next && g_CurrentLevelInfo->SecretNext)
			NewInfo = P_FindLevelByNameEx(g_CurrentLevelInfo->SecretNext, NULL);
		
		// Normal Exit?
		if (!NewInfo && g_CurrentLevelInfo->NormalNext)
			NewInfo = P_FindLevelByNameEx(g_CurrentLevelInfo->NormalNext, NULL);
		
		// Was able to find the level?
		if (NewInfo)
			P_ExLoadLevel(NewInfo, false);
		// Otherwise it was a failure, try something else
		else
		{
			// Back to the titlescreen!
			gamestate = GS_DEMOSCREEN;
		}
#if 0
	}
	else
	{
		// not in demo because demo have the mapcommand on it
		if (!demoplayback)
		{
			if (cv_deathmatch.value)
				G_InitNew(gameskill, G_BuildMapName(gameepisode, wminfo.next + 1), 1);
			else
				G_InitNew(gameskill, G_BuildMapName(gameepisode, wminfo.next + 1), 0);
			/*
			   if (cv_deathmatch.value == 0)
			   // don't reset player between maps
			   COM_BufAddText(va
			   ("map \"%s\" -noresetplayers\n",
			   G_BuildMapName(gameepisode, wminfo.next + 1)));
			   else
			   // resetplayer in deathmatch for more equality
			   COM_BufAddText(va("map \"%s\"\n", G_BuildMapName(gameepisode, wminfo.next + 1))); */
		}
#endif
		
	gameaction = ga_nothing;
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
void G_LoadGame(int slot)
{
}

#define VERSIONSIZE             16

void G_DoLoadGame(int slot)
{
#if 0
	int length;
	char vcheck[VERSIONSIZE];
	char savename[255];
	
	sprintf(savename, savegamename, slot);
	
	length = FIL_ReadFile(savename, &savebuffer);
	if (!length)
	{
		CONL_PrintF("Couldn't read file %s", savename);
		return;
	}
	// skip the description field
	save_p = savebuffer + SAVESTRINGSIZE;
	
	memset(vcheck, 0, sizeof(vcheck));
	sprintf(vcheck, "version %i", VERSION);
	if (strcmp(save_p, vcheck))
	{
		M_StartMessage("Save game from different version\n\nPress ESC\n", NULL, MM_NOTHING);
		return;					// bad version
	}
	save_p += VERSIONSIZE;
	
	if (demoplayback)			// reset game engine
		G_StopDemo();
		
	//added:27-02-98: reset the game version
	G_Downgrade(VERSION);
	
	paused = false;
	automapactive = false;
	
	// dearchive all the modifications
	if (!P_LoadGame())
	{
		M_StartMessage("savegame file corrupted\n\nPress ESC\n", NULL, MM_NOTHING);
		Command_ExitGame_f();
		Z_Free(savebuffer);
		return;
	}
	
	gameaction = ga_nothing;
	gamestate = GS_LEVEL;
	g_Splits[0].Display = g_Splits[0].Console;
	
	// done
	Z_Free(savebuffer);
	
	multiplayer = playeringame[1];
	//if (playeringame[1] && !netgame)
	//	CV_SetValue(&cv_splitscreen, 1);
		
	if (setsizeneeded)
		R_ExecuteSetViewSize();
		
	// draw the pattern into the back screen
	R_FillBackScreen();
	CON_ToggleOff();
#endif
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 uint8_t text string
//
void G_SaveGame(int slot, char* description)
{
}

void G_DoSaveGame(int savegameslot, char* savedescription)
{
#if 0
	char name2[VERSIONSIZE];
	char description[SAVESTRINGSIZE];
	int length;
	char name[256];
	
	gameaction = ga_nothing;
	
	sprintf(name, savegamename, savegameslot);
	
	gameaction = ga_nothing;
	
	save_p = savebuffer = (uint8_t*)malloc(SAVEGAMESIZE);
	if (!save_p)
	{
		CONL_PrintF("No More free memory for savegame\n");
		return;
	}
	
	strcpy(description, savedescription);
	description[SAVESTRINGSIZE] = 0;
	WRITEMEM(save_p, description, SAVESTRINGSIZE);
	memset(name2, 0, sizeof(name2));
	sprintf(name2, "version %i", VERSION);
	WRITEMEM(save_p, name2, VERSIONSIZE);
	
	P_SaveGame();
	
	length = save_p - savebuffer;
	if (length > SAVEGAMESIZE)
		I_Error("Savegame buffer overrun");
	FIL_WriteFile(name, savebuffer, length);
	free(savebuffer);
	
	gameaction = ga_nothing;
	
	players[g_Splits[0].Console].message = GGSAVED;
	
	// draw the pattern into the back screen
	R_FillBackScreen();
#endif

#if 0
	char ExtFileName[MAX_WADPATH];
	size_t FileLen = 0;
	
	CONL_PrintF("G_DoSaveGame: Saving the game...\n");
	
	// Can't save when NOT playing
	if (!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
	{
		CONL_PrintF("G_DoSaveGame: You can't save the game if you are not inside of a game!\n");
		return;
	}
	// Setup
	memset(ExtFileName, 0, sizeof(ExtFileName));
	gameaction = ga_nothing;
	
	if (P_SaveGameEx(savedescription, ExtFileName, MAX_WADPATH, &FileLen, &savebuffer))
	{
		if (savebuffer)
		{
			FIL_WriteFile(ExtFileName, savebuffer, FileLen);
			Z_Free(savebuffer);
			savebuffer = NULL;
		}
		
		gameaction = ga_nothing;
		
		CONL_PrintF("G_DoSaveGame: Game saved!\n");
		R_FillBackScreen();
	}
	
	gameaction = ga_nothing;
#endif
}

//added:03-02-98:
//
//  'Downgrade' the game engine so that it is compatible with older demo
//   versions. This will probably get harder and harder with each new
//   'feature' that we add to the game. This will stay until it cannot
//   be done a 'clean' way, then we'll have to forget about old demos..
//
bool_t G_Downgrade(int version)
{
	int i;
	
	if (version < 109)
		return false;
	
	//hmmm.. first time I see an use to the switch without break...
	switch (version)
	{
		case 109:
			// Boris : for older demos, initalise the new skincolor value
			//         also disable the new preferred weapons order.
			for (i = 0; i < 4; i++)
			{
				players[i].skincolor = i % MAXSKINCOLORS;
				players[i].originalweaponswitch = true;
			}					//eof Boris
		case 110:
		case 111:
			//added:16-02-98: make sure autoaim is used for older
			//                demos not using mouse aiming
			for (i = 0; i < MAXPLAYERS; i++)
				players[i].autoaim_toggle = true;
				
		default:
			break;
	}
	
	//SoM: 3/17/2000: Demo compatability
	if (version < 129)
	{
		boomsupport = 0;
		allow_pushers = 0;
		variable_friction = 0;
	}
	else
	{
		boomsupport = 1;
		allow_pushers = 1;
		variable_friction = 1;
	}
	
	// always true now, might be false in the future, if couldn't
	// go backward and disable all the features...
	demoversion = version;
	return true;
}

#if 0
/* G_DoPlayDemo() -- Plays A demo */
void G_DoPlayDemo(char* defdemoname)
{
	skill_t skill;
	int i, j, episode, map;
	
	WL_WADEntry_t* Entry;
	WL_ES_t* Stream;
	
	/* Stop playing old demo */
	if (g_CurDemoInfo)
		Z_Free(g_CurDemoInfo);
	g_CurDemoInfo = NULL;
	
	/* Try playing by entry */
	Entry = WL_FindEntry(NULL, 0, defdemoname);
	
	// Found?
	if (Entry)
	{
		// Open Stream
		Stream = WL_StreamOpen(Entry);
		
		// Found!
		if (Stream)
		{
			// Allocate
			g_CurDemoInfo = Z_Malloc(sizeof(*g_CurDemoInfo), PU_STATIC, NULL);
			
			// Fill
			g_CurDemoInfo->Entry = Entry;
			g_CurDemoInfo->EntryStream = Stream;
			g_CurDemoInfo->DemoVersion = WL_Sru8(Stream);
		}
	}
	
	/* Begin demo playback now */
	if (g_CurDemoInfo)
	{
		// Inform the watcher
		demoplayback = true;
		CONL_PrintF("Playing back %i.%02i\n",
				g_CurDemoInfo->DemoVersion / 100, g_CurDemoInfo->DemoVersion % 100);
		
		// Load Demo Information
	}
	
#if 0
//
// load demo file / resource
//

	//it's an internal demo
	if ((i = W_CheckNumForName(defdemoname)) == INVALIDLUMP)
	{
		FIL_DefaultExtension(defdemoname, ".lmp");
		if (!FIL_ReadFile(defdemoname, &demobuffer))
		{
			CONL_PrintF("\2ERROR: couldn't open file '%s'.\n", defdemoname);
			goto no_demo;
		}
		demo_p = demobuffer;
	}
	else
		demobuffer = demo_p = W_CacheLumpNum(i, PU_STATIC);
		
//
// read demo header
//

	gameaction = ga_nothing;
	demoversion = READBYTE(demo_p);
	if (demoversion < 109)
	{
		CONL_PrintF("\2ERROR: demo version too old.\n");
		demoversion = VERSION;
		if (demobuffer)
			Z_Free(demobuffer);
		demobuffer = NULL;
no_demo:
#endif
		gameaction = ga_nothing;
		return;
#if 0
	}
	
	if (demoversion < VERSION)
		CONL_PrintF("\2Demo is from an older game version\n");
		
	CONL_PrintF("Demo is from %i.i\n", demoversion / 100, demoversion % 100);
	
	skill = *demo_p++;
	episode = *demo_p++;
	map = *demo_p++;
	if (demoversion < 127)
		// push it in the console will be too late set
		cv_deathmatch.value = *demo_p++;
	else
		demo_p++;
		
	if (demoversion < 128)
		// push it in the console will be too late set
		cv_respawnmonsters.value = *demo_p++;
	else
		demo_p++;
		
	if (demoversion < 128)
	{
		// push it in the console will be too late set
		cv_fastmonsters.value = *demo_p++;
		cv_fastmonsters.func();
	}
	else
		demo_p++;
		
	nomonsters = *demo_p++;
	
	//added:08-02-98: added displayplayer because the status bar links
	// to the display player when playing back a demo.
	g_Splits[0].Display = g_Splits[0].Console = *demo_p++;
	
	//added:11-01-98:
	//  support old v1.9 demos with ONLY 4 PLAYERS ! Man! what a shame!!!
	if (demoversion == 109)
	{
		for (i = 0; i < 4; i++)
			playeringame[i] = *demo_p++;
	}
	else
	{
		if (demoversion < 128)
		{
			cv_timelimit.value = *demo_p++;
			cv_timelimit.func();
		}
		else
			demo_p++;
			
		if (demoversion < 113)
		{
			for (i = 0; i < 8; i++)
				playeringame[i] = *demo_p++;
		}
		else
		{
			if (demoversion >= 131)
				multiplayer = *demo_p++;
				
			for (i = 0; i < 32; i++)
				playeringame[i] = *demo_p++;
		}
#if MAXPLAYERS>32
#error Please add support for old lmps
#endif
	}
	
	if (demoversion < 131)
	{
		j = 0;
		
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
				j++;
				
		multiplayer = j > 1;
	}
	
	memset(oldcmd, 0, sizeof(oldcmd));
	
	// don't spend a lot of time in loadlevel
	if (demoversion < 127)
	{
		precache = false;
		G_InitNew(skill, G_BuildMapName(episode, map), true);
		precache = false;		// GhostlyDeath -- was true
		CON_ToggleOff();		// will be done at the end of map command
	}
	else
		// wait map command in the demo
		gamestate = wipegamestate = GS_WAITINGPLAYERS;
		
	if (g_SplitScreen)
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		{
			g_Splits[i].Display = i;
			g_Splits[i].Console = i;
		}
		
	demoplayback = true;
	M_LockGameCVARS();
	DC_SetMenuGameOptions(1);
	DC_SetDemoOptions(demoversion);
#endif
}
#endif

#if 0
//
// G_TimeDemo
//             NOTE: name is a full filename for external demos
//
static int restorecv_vidwait;

void G_TimeDemo(char* name)
{
	int i;
	
	if (g_SplitScreen > 0)
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		{
			g_Splits[i].Display = i;
			g_Splits[i].Console = i;
		}
	nodrawers = M_CheckParm("-nodraw");
	noblit = M_CheckParm("-noblit");
	restorecv_vidwait = cv_vidwait.value;
	if (cv_vidwait.value)
		CV_Set(&cv_vidwait, "0");
	timingdemo = true;
	singletics = true;
	framecount = 0;
	demostarttime = I_GetTime();
	G_DeferedPlayDemo(name);
}
#endif

void G_DoneLevelLoad(void)
{
	// GhostlyDeath <February 8, 2012> -- uint64_t to double not impl on MSVC6
	//CONL_PrintF("Load Level in %f sec\n", (float)(I_GetTime() - demostarttime) / TICRATE);
	framecount = 0;
	demostarttime = I_GetTime();
}

/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

#if 0
// reset engine variable set for the demos
// called from stopdemo command, map command, and g_checkdemoStatus.
void G_StopDemo(void)
{
	if (demobuffer)
		Z_Free(demobuffer);
	demobuffer = NULL;
	demoplayback = false;
	timingdemo = false;
	singletics = false;
	
	G_Downgrade(VERSION);
	
	gamestate = wipegamestate = GS_NULL;
}

bool_t G_CheckDemoStatus(void)
{
	if (timingdemo)
	{
		int time;
		float f1, f2;
		
		time = I_GetTime() - demostarttime;
		if (!time)
			return true;
		G_StopDemo();
		timingdemo = false;
		f1 = time;
		f2 = framecount * TICRATE;
		CONL_PrintF("timed %i gametics in %i realtics\n" "%f secondes, %f avg fps\n", leveltime, time, f1 / TICRATE, f2 / f1);
		if (restorecv_vidwait != cv_vidwait.value)
			CV_SetValue(&cv_vidwait, restorecv_vidwait);
		D_AdvanceDemo();
		return true;
	}
	else if (demoplayback)
	{
		if (singledemo)
			I_Quit();
		G_StopDemo();
		D_AdvanceDemo();
		return true;
	}
	else if (demorecording)
	{
		RDF_DEMO_EndRecording();
#if 0
		RDF_WriteEndDemo();
		
		/**demo_p++ = DEMOMARKER;*/
		FIL_WriteFile(demoname, demobuffer, demo_p - demobuffer);
		if (demobuffer)
			Z_Free(demobuffer);
		demobuffer = NULL;
		demorecording = false;
		
		CONL_PrintF("\2Demo %s recorded\n", demoname);
		return true;
		//I_Quit ();
#endif
	}
	
	return false;
}
#endif

