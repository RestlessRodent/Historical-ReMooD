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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION:
//      game loop functions, events handling

/* Includes */
//#include "doomdef.h"
#include "doomtype.h"

//#include "console.h"
//#include "dstrings.h"
//#include "d_main.h"
//#include "sn.h"
//#include "d_netcmd.h"
//#include "f_finale.h"
//#include "p_setup.h"
//#include "p_saveg.h"
//#include "i_system.h"
//#include "wi_stuff.h"

//#include "m_random.h"
//#include "p_local.h"
//#include "p_tick.h"
//#include "r_data.h"
//#include "r_draw.h"
//#include "r_main.h"
//#include "r_sky.h"
//#include "s_sound.h"
//#include "g_game.h"
//#include "g_state.h"
//#include "g_input.h"
//#include "m_cheat.h"
//#include "m_misc.h"
//#include "m_menu.h"
//#include "m_argv.h"
//#include "hu_stuff.h"
//#include "st_stuff.h"

//#include "w_wad.h"
//#include "z_zone.h"
//#include "i_video.h"
//#include "p_inter.h"
//#include "p_info.h"
//#include "p_demcmp.h"
//#include "b_bot.h"
//#include "st_stuff.h"


// added 8-3-98 increse savegame size from 0x2c000 (180kb) to 512*1024
#define SAVEGAMESIZE    (512*1024)
#define SAVESTRINGSIZE  24

bool_t G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum);

void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);

uint8_t gameepisode;
uint8_t gamemap;
char gamemapname[GAMEMAPNAMESIZE];	// an external wad filename

gamemode_t gamemode = indetermined;	// Game Mode - identify IWAD as shareware, retail etc.
gamemission_t gamemission = doom;
bool_t modifiedgame;			// Set if homebrew PWAD stuff has been added.

bool_t paused;

bool_t timingdemo;				// if true, exit with report on completion
bool_t nodrawers;				// for comparative timing purposes
bool_t noblit;					// for comparative timing purposes
tic_t demostarttime;			// for comparative timing purposes

bool_t netgame;					// only true if packets are broadcast
bool_t multiplayer;
bool_t serverside;
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

wbstartstruct_t wminfo;			// parms for world map / intermission

uint8_t* savebuffer;

// changed to 2d array 19990220 by Kin
char player_names[MAXPLAYERS][MAXPLAYERNAME];
char team_names[MAXPLAYERS][MAXPLAYERNAME * 2];

mobj_t* bodyque[BODYQUESIZE];
int bodyqueslot;

void* statcopy;					// for statistics driver

//
//  Clip the console player mouse aiming to the current view,
//  also returns a signed char for the player ticcmd if needed.
//  Used whenever the player view pitch is changed manually
//
//added:22-02-98:
//changed:3-3-98: do a angle limitation now
int16_t G_ClipAimingPitch(int32_t* aiming)
{
	int32_t limitangle;
	
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

bool_t P_CanUseWeapon(player_t* const a_Player, const PI_wepid_t a_Weapon);

/* NextWeapon() -- Finds the next weapon in the chain */
// This is for PrevWeapon and NextWeapon
// Rewritten for RMOD Support!
// This uses the fields in PI_wep_t for ordering info
uint8_t NextWeapon(player_t* player, int step)
{
	size_t g, w, fw, BestNum;
	int32_t s, StepsLeft, StepsAdd, BestDiff, ThisDiff;
	size_t MostOrder, LeastOrder;
	bool_t Neg;
	PI_wep_t* weapons;
	
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


// a_ChatPrefix -- Chat prefix for player screens
static const char* const a_ChatPrefix[4] =
{
	"", "\x4", "\x5", "\x6"
};

/* GS_HandleExtraCommands() -- Handles extra commands */
static void GS_HandleExtraCommands(ticcmd_t* const a_TicCmd, const int32_t a_PlayerNum)
{
	const uint8_t* Rp, *Rb, *Re, *Next;
	uint8_t Command;
	
	int i, j, k, l;
	
	D_Prof_t* Profile;
	player_t* Player;
	mobj_t* Mo;
	
	uint32_t u32[6];
	int32_t i32[6];
	uint16_t u16[6];
	uint8_t u8[6];
	int8_t SplitNum;
	char NameBuf[MAXTCCBUFSIZE + MAXTCCBUFSIZE + 1];
	char AltBuf[MAXTCCBUFSIZE + MAXTCCBUFSIZE + 1];
	bool_t OK, LegalMove;
	
	/* Get pointer base */
	if (a_TicCmd->Ctrl.Type == 1)
	{
		Rb = Rp = a_TicCmd->Ext.DataBuf;
		Re = (const uint8_t*)((uintptr_t)(Rb + a_TicCmd->Ext.DataSize));
	}
	else
	{
		Rb = Rp = a_TicCmd->Std.DataBuf;
		Re = (const uint8_t*)((uintptr_t)(Rb + a_TicCmd->Std.DataSize));
	}
	
	/* Constantly Read Bits */
	Next = Rp;
	do
	{
		// Read Next Command
		Rp = Next;
		Command = ReadUInt8((uint8_t**)&Rp);
		
		// Don't overflow! untrusted!
		if (Command <= 0 || Command >= NUMDTCT)
			break;
		
		// Determine next command to read
		Next = Rp + c_TCDataSize[Command];
			
		// Not big enough?
		if ((uintptr_t)(Rp + c_TCDataSize[Command]) > (uintptr_t)(Re))
			break;
			
		// Clear
		memset(NameBuf, 0, sizeof(NameBuf));
		memset(AltBuf, 0, sizeof(AltBuf));
		
		// Which command?
		switch (Command)
		{
				// Simple Network Commands
			case DTCT_SNJOINPLAYER:
			case DTCT_SNQUITREASON:
			case DTCT_SNCLEANUPHOST:
			case DTCT_SNJOINHOST:
			case DTCT_SNPARTPLAYER:
			case DTCT_SNJOINPORT:
			case DTCT_SNCHATFRAG:
			case DTCT_SNPORTSETTING:
				SN_HandleGT(Command, &Rp);
				break;
				
				// Map Changes
			case DTCT_MAPCHANGE:
				// Read Data
				u8[0] = ReadUInt8((const uint8_t**)&Rp);
				
				for (i = 0; i < 8; i++)
					NameBuf[i] = ReadUInt8((const uint8_t**)&Rp);
				
				// Resetting players?
				if (u8[0])
					for (i = 0; i < MAXPLAYERS; i++)
						if (playeringame[i])
							G_ResetPlayer(&players[i]);
				
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
				u32[0] = LittleReadUInt32((const uint32_t**)&Rp);
				i32[0] = LittleReadInt32((const int32_t**)&Rp);
				
				// Change Variable
				P_XGSSetValue(true, u32[0], i32[0]);
				break;
			
				// Change Monster Team
			case DTCT_XCHANGEMONSTERTEAM:
				// Read Data
				u32[0] = LittleReadUInt32((const uint32_t**)&Rp);
				u8[0] = ReadUInt8((const uint32_t**)&Rp);
				
				// Bounds OK?
				if (u32[0] >= 0 && u32[0] < MAXPLAYERS)
					if (playeringame[u32[0]])
						P_ChangeCounterOp(&players[u32[0]], u8[0]);
				break;
				
				// Morph Player
			case DTCT_XMORPHPLAYER:
				// Read Data
				u32[0] = LittleReadUInt32((const uint32_t**)&Rp);
				for (i = 0; i < MAXPLAYERNAME; i++)
					NameBuf[i] = ReadUInt8((const uint8_t**)&Rp);
					
				// Bounds OK?
				if (u32[0] >= 0 && u32[0] < MAXPLAYERS)
					if (playeringame[u32[0]])
						P_MorphObjectClass(players[u32[0]].mo, INFO_GetTypeByName(NameBuf));
				break;
				
			default:
				CONL_PrintF("Unknown command %i.\n", Command);
				Command = 0;
				break;
		}
	} while (Command != 0);
}

extern tic_t g_WatchTic;

/* G_CalcSyncCode() -- Calculates the current sync code for the game */
uint32_t G_CalcSyncCode(const bool_t a_Debug)
{
	uint32_t i, Code;
	PI_wep_t* Weap;
	
	/* Init */
	Code = 0;
	
	/* Base */
	Code |= (uint32_t)(gametic & UINT32_C(0x7FFFFFFF));
	i = (uint32_t)P_GetRandIndex();
	Code ^= (i << UINT32_C(24)) | (i);
	
	// Debugging
	if (a_Debug)
	{
		CONL_PrintF("gametic = %u\n", (unsigned)gametic);
		CONL_PrintF("prandex = %u\n", P_GetRandIndex());
	}
	
	/* Run through players */
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// Not playing? Ignore
		if (!playeringame[i])
			continue;
		
		// Flip single bit
		Code ^= (UINT32_C(1) << (i & UINT32_C(31)));
		
		if (a_Debug)
			CONL_PrintF("pig[%02i] = %i\n", i, playeringame[i]);
		
		// Position, if object exists and playing
		if (gamestate == GS_LEVEL)
		{
			// Player health
			Code += players[i].health;
			Code -= players[i].armorpoints;
			
			// Add Ammo
			if (players[i].readyweapon >= 0 && players[i].readyweapon < NUMWEAPONS)
			{
				Weap = players[i].weaponinfo[players[i].readyweapon];
				
				if (Weap->ammo >= 0 && Weap->ammo < NUMAMMO)
					Code += (players[i].ammo[Weap->ammo] & INT32_C(0x7F)) << INT32_C(6);
			}
			
			// Attack Held Down
			if (players[i].attackdown)
				Code ^= 0x20;
			
			// Bobbing
			Code ^= players[i].FlatBob;
			Code ^= players[i].psprites[0].sx;
			Code ^= players[i].psprites[0].sy;
			
			// By player object
			if (players[i].mo)
			{
				Code ^= players[i].mo->x;
				Code ^= players[i].mo->y;
				Code ^= players[i].mo->z;
				Code ^= players[i].mo->momx >> FRACBITS;
				Code ^= players[i].mo->momy >> FRACBITS;
				Code ^= players[i].mo->momz >> FRACBITS;
				
				if (a_Debug)
				{
					CONL_PrintF("p[%02i].x = %f (%08x)\n", i, FIXED_TO_FLOAT(players[i].mo->x), players[i].mo->x);
					CONL_PrintF("p[%02i].y = %f (%08x)\n", i, FIXED_TO_FLOAT(players[i].mo->y), players[i].mo->y);
					CONL_PrintF("p[%02i].z = %f (%08x)\n", i, FIXED_TO_FLOAT(players[i].mo->z), players[i].mo->z);
				}
			}
		
			// Otherwise, flip another bit
			else
				Code ^= (UINT32_C(1) << ((i + 1) & UINT32_C(31)));
		}
	}
	
	/* Return generated code */
	return Code;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(void)
{
	uint32_t i;
	ticcmd_t* cmd;
	ticcmd_t GlobalCmd;
	
	uint32_t NowCode, DemoCode;
	static bool_t DidPP;
	
	/* Save game on first tic while recording */
	if (demorecording)
		if (gametic == 0)
			G_EncodeSaveGame();
		
	/* Process ++ args? */
	if (!DidPP && gametic == 0)
	{
		M_PushSpecialPlusParameters();
		DidPP = 1;
	}
	
	/* Init Code */
	NowCode = DemoCode = 0;
		
	/* Pre-Tic */
	// Read start of tic before reborns and game actions, because ReadStarTic()
	// might load a savegame where the action is going to take place.
	if (demoplayback)
	{
		G_ReadStartTic(&DemoCode);
		
		// Check the status of the demo
		G_CheckDemoStatus();
		
		// If there is no demo playing, then return from this func
		// Otherwise, there will be a false demo desync at the very end of the
		// demo.
		if (!demoplayback)
			return;
	}
	
	if (demorecording)
		G_WriteStartTic(NowCode);
	
	// start network tick
	SN_StartTic(gametic);
	
	/* Global Commands */
	// Clear
	memset(&GlobalCmd, 0, sizeof(GlobalCmd));
	
	// Read Global Tic Commands
	if (demoplayback)
		G_ReadDemoGlobalTicCmd(&GlobalCmd);
	else
		SN_Tics(&GlobalCmd, false, -1);
	
	// Write Global Tic Commands
	if (demorecording)
		G_WriteDemoGlobalTicCmd(&GlobalCmd);
	SN_Tics(&GlobalCmd, true, -1);
	
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
				SN_Tics(cmd, false, i);
			
			// Write Command
			if (demorecording)
				G_WriteDemoTiccmd(cmd, i);
			SN_Tics(cmd, true, i);
			
			// Copy Ping
			players[i].Ping = cmd->Ctrl.Ping;
		}
		
	/* Calculate the current sync code */
	NowCode = G_CalcSyncCode((devparm && gametic <= 1));
		
	/* Post-Tic */
	if (demoplayback)
		G_ReadEndTic(&DemoCode);
	
	if (demorecording)
		G_WriteEndTic(NowCode);
	
	// Networking
	SN_SyncCode(gametic, NowCode);
	
	/* If playing a demo, check code comparison */
	// Also make sure the demo supports the ReMooD sync code, otherwise there
	// will always be a false desync message.
	if (demoplayback && G_UseDemoSyncCode())
		if (NowCode != DemoCode)
		{
			CONL_PrintF("{2Demo desync! (tic %u, %08x != %08x)\n", (unsigned int)gametic, NowCode, DemoCode);
			
			if (devparm)
				G_CalcSyncCode(true);
			
			// Show message, do warning, etc.
		}
	
	/* Do reborns and such */
	// This used to be between the start tic and end tic, but that would cause
	// a problem of sync codes not being correct, etc.
	
	// Do reborns
	if (gamestate == GS_LEVEL)
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i])
			{
				if (players[i].playerstate == PST_REBORN)
				{
					// Player is on monster's side
					if (P_GMIsCounter() && players[i].CounterOpPlayer)
						P_ControlNewMonster(&players[i]);
						
					// Otherwise make player
					else
						G_DoReborn(i);
				}
				
				if (players[i].st_inventoryTics)
					players[i].st_inventoryTics--;
			}
	
	// Change game state
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
	
	/* Handle Commands */
	// Process Global Commands
	if (GlobalCmd.Ctrl.Type >= 1)
		GS_HandleExtraCommands(&GlobalCmd, -1);
		
	// Per Player Commands
	for (i = 0; i < MAXPLAYERS; i++)
		if ((playeringame[i] || i == 0) && !dedicated)
			GS_HandleExtraCommands(cmd, i);
	
	/* Remove defunct XPlayer */
	//D_XNetClearDefunct();
	
	// Set new time
	g_DemoTime = gametic;
	
	// do main actions
	switch (gamestate)
	{
		case GS_LEVEL:
			//B_GHOST_Ticker();
			P_Ticker();			// tic the game
			ST_Ticker();
			ST_TickerEx();
			break;
			
		case GS_INTERMISSION:
			WI_Ticker();
			break;
			
		case GS_FINALE:
			F_Ticker();
			break;
			
		case GS_WAITFORJOINWINDOW:
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
	
	// GhostlyDeath <December 28, 2012> -- Inventory reduction
	if (P_XGSVal(PGS_PLREDUCEINVENTORY))
		for (i = 0; i < p->inventorySlotNum; i++)
			if (p->inventory[i].count > 1)
				p->inventory[i].count = 1;
	
	p->weaponinfo = wpnlev1info;	// cancel power weapons
	p->mo->flags &= ~MF_SHADOW;	// cancel invisibility
	p->extralight = 0;			// cancel gun flashes
	p->fixedcolormap = 0;		// cancel ir gogles
	p->damagecount = 0;			// no palette changes
	p->bonuscount = 0;
	p->PalChoice = 0;
	
	p->KeyCards[0] = p->KeyCards[1] = NULL;
	
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
	int i, j;
	uint16_t frags[MAXPLAYERS];
	int killcount;
	int itemcount;
	int secretcount;
	uint16_t addfrags;
	bool_t* weaponowned;
	PI_wepid_t* FavoriteWeapons;
	int* ammo;
	int* maxammo;
	uint32_t FraggerID;
	tic_t SuicideDelay;
	bool_t* RandGuns;
	
	D_Prof_t* PEp;
	SN_Port_t* Port;
	
	//from Boris
	int32_t skincolor, VTeamColor;
	//char favoritweapon[NUMWEAPONS];
	bool_t originalweaponswitch;
	bool_t autoaim;
	int skin;					//Fab: keep same skin
	int32_t TotalFrags, TotalDeaths;
	bool_t Given, CounterOp;
	
	PEp = players[player].ProfileEx;
	Port = players[player].Port;
	
	memcpy(frags, players[player].frags, sizeof(frags));
	addfrags = players[player].addfrags;
	killcount = players[player].killcount;
	itemcount = players[player].itemcount;
	secretcount = players[player].secretcount;
	TotalFrags = players[player].TotalFrags;
	TotalDeaths = players[player].TotalDeaths;
	FraggerID = players[player].FraggerID;
	SuicideDelay = players[player].SuicideDelay;
	
	FavoriteWeapons = players[player].FavoriteWeapons;
	weaponowned = players[player].weaponowned;
	ammo = players[player].ammo;
	maxammo = players[player].maxammo;
	
	//from Boris
	VTeamColor = players[player].VTeamColor;
	skincolor = players[player].skincolor;
	originalweaponswitch = players[player].originalweaponswitch;
	//memcpy(favoritweapon, players[player].favoritweapon, NUMWEAPONS);
	autoaim = players[player].autoaim_toggle;
	skin = players[player].skin;
	CounterOp = players[player].CounterOpPlayer;
	
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
	players[player].SuicideDelay = SuicideDelay;
	
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
	players[player].CounterOpPlayer = CounterOp;
	players[player].VTeamColor = VTeamColor;
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
				// Only starting with melee weapons
				if (P_XGSVal(PGS_PLSPAWNWITHMELEEONLY))
					if (!(p->weaponinfo[i]->WeaponFlags & WF_ISMELEE))
						continue;
				
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
	players[player].Port = Port;
	
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
		
		// Melee Only Cancel?
		if (P_XGSVal(PGS_PLSPAWNWITHMELEEONLY))
			if (!(p->weaponinfo[i]->WeaponFlags & WF_ISMELEE))
				p->weaponowned[i] = Given = false;
		
		// Gave something? Then give max ammo
		if (Given)
			if (p->weaponinfo[i]->ammo >= 0 && p->weaponinfo[i]->ammo < NUMAMMO)
				p->ammo[p->weaponinfo[i]->ammo] = p->maxammo[p->weaponinfo[i]->ammo];
	}
	
	/* Spawn with a single random gun */
	if (P_XGSVal(PGS_PLSPAWNWITHRANDOMGUN))
	{
		// Allocate Array
		RandGuns = Z_Malloc(sizeof(*RandGuns) * NUMWEAPONS, PU_STATIC, NULL);
		
		// Go through weapons and fill in owned guns
		for (j = 0, i = 0; i < NUMWEAPONS; i++)
			if (p->weaponowned[i])
				RandGuns[j++] = i;
		
		// Make sure he has at least one gun!
		if (j > 0)
		{
			// Select a random gun from this list
			i = abs(P_SignedRandom()) % j;
			
			// Go through all weapons again
			for (j = 0; j < NUMWEAPONS; j++)
			{
				// Gun we want to keep
				if (RandGuns[i] == j)
					continue;
				
				// Remove ownership of it
				p->weaponowned[j] = false;
				
				// Remove Ammo
				if (p->weaponinfo[j]->ammo >= 0 && p->weaponinfo[j]->ammo < NUMAMMO)
					if (p->weaponinfo[RandGuns[i]]->ammo != p->weaponinfo[j]->ammo)
						p->ammo[p->weaponinfo[j]->ammo] = 0;
			}
		}
		
		// Cleanup
		Z_Free(RandGuns);
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
	if (!P_XGSVal(PGS_CONEWGAMEMODES) && P_GMIsTeam())
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
	
	// GhostlyDeath <April 21, 2012> -- Check against radius
	if (/*a_NoFirstMo &&*/ P_XGSVal(PGS_CORADIALSPAWNCHECK))
	{
		if (!P_CheckPosRadius(x, y, FIXEDT_C(20)))
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

/* GS_CheckSpotArea() -- Checks the area a spot belong in */
// Sees if there is no path there, not walkable, can't be seen there, etc.
static bool_t GS_CheckSpotArea(const fixed_t a_X1, const fixed_t a_Y1, const fixed_t a_X2, const fixed_t a_Y2)
{
#define BLOCKSIZE FIXEDT_C(16)
	
	/* See if a straight line can be drawn between these two spots */
	if (!P_CheckSightLine(
				a_X1,
				a_Y1,
				a_X2,
				a_Y2
			))
		return false;
	
	/* Draw another line from the soure to destination */
	if (!P_PathTraverse(
				a_X1,
				a_Y1,
				a_X2,
				a_Y2,
				PT_ADDLINES,
				GS_ClusterTraverser,
				NULL
			))
		return false;
	
	/* Draw line from thing corner (corss section BL to TR) */
	if (!P_PathTraverse(
				a_X2 - BLOCKSIZE,
				a_Y2 - BLOCKSIZE,
				a_X2 + BLOCKSIZE,
				a_Y2 + BLOCKSIZE,
				PT_ADDLINES | PT_ADDTHINGS,
				GS_ClusterTraverser,
				NULL
			))
		return false;
	
	/* Draw line from thing corner (corss section TL to BR) */
	if (!P_PathTraverse(
				a_X2 - BLOCKSIZE,
				a_Y2 + BLOCKSIZE,
				a_X2 + BLOCKSIZE,
				a_Y2 - BLOCKSIZE,
				PT_ADDLINES | PT_ADDTHINGS,
				GS_ClusterTraverser,
				NULL
			))
		return false;
	
	/* Nothing is here and it is reachable */
	return true;
#undef BLOCKSIZE
}

/* G_ClusterSpawnPlayer() -- Spawns player in cluster spot */
bool_t G_ClusterSpawnPlayer(const int PlayerID, const bool_t a_CheckOp)
{
#define MAXFAILURES 128
	mapthing_t** Spots;
	size_t NumSpots, i, s, j, f;
	int32_t x, y, bx, by;
	mapthing_t OrigThing, FakeThing;
	subsector_t* SubS;
	bool_t RandomSpot;
	bool_t* Tried;
	bool_t PreDiamond, DMMode, TeamMode;
	
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
	
	/* Deathmatch mode? */
	DMMode = P_GMIsDM();
	TeamMode = P_GMIsTeam() && P_XGSVal(PGS_CONEWGAMEMODES);
	
	/* Which spots to prefer? */
	if (TeamMode && !a_CheckOp)
	{
		// Determine team player is on
		i = players[PlayerID].VTeamColor;
		
		// Spawn at this location
		PreDiamond = true;
		RandomSpot = true;
		Spots = g_TeamStarts[i];
		NumSpots = MAXPLAYERS;
		Tried = Z_Malloc(sizeof(*Tried) * NumSpots, PU_STATIC, NULL);
	}
	
	// Deathmatch
	else if (DMMode || (!DMMode && a_CheckOp) || (TeamMode && a_CheckOp))
	{
		PreDiamond = false;
		RandomSpot = true;
		Spots = deathmatchstarts;
		NumSpots = numdmstarts;
		Tried = Z_Malloc(sizeof(*Tried) * NumSpots, PU_STATIC, NULL);
	}
	
	// Coop
	else if (!DMMode || (DMMode && a_CheckOp))
	{
		PreDiamond = true;
		RandomSpot = false;
		Spots = playerstarts;
		NumSpots = MAXPLAYERS;
		Tried = NULL;
	}
	
	/* Determine offset base */
	if (a_CheckOp || DMMode)
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
			f = 0;
			do
			{
				f++;
				i = P_Random() % NumSpots;
				
				// Too many failures?
				if (f >= MAXFAILURES)
					break;
				
				// All tried?
				for (j = 0; j < NumSpots; j++)
					if (!Tried[j])
						break;
				if (j >= NumSpots)
					break;
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
				if (!DMMode)
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
				
				// Check the area to and around the wanted spot
				if (!GS_CheckSpotArea(
						((fixed_t)OrigThing.x) << FRACBITS,
						((fixed_t)OrigThing.y) << FRACBITS,
						((fixed_t)FakeThing.x) << FRACBITS,
						((fixed_t)FakeThing.y) << FRACBITS
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
#undef MAXFAILURES
}

/* G_DisplaceSpawnPlayer() -- Spawns player next to another player */
bool_t G_DisplaceSpawnPlayer(const int32_t a_PlayerID)
{
	int32_t i, x, y;
	bool_t DMMode, Teams;
	mapthing_t FakeSpot;
	
	/* Deathmatch mode? */
	Teams = P_GMIsTeam();
	DMMode = P_GMIsDM();
	
	// Non-team DM, always fail
	if (DMMode && !Teams)
		return false;
	
	/* Go through all players */
	// Go from last to first, since later players would be far away
	for (i = MAXPLAYERS - 1; i >= 0; i--)
	{
		// Not playing? ignore
		if (!playeringame[i])
			continue;
		
		// This is ourself
		if (i == a_PlayerID)
			continue;
			
		// Not on same team?
		if (DMMode && Teams && !P_MobjOnSameTeam(players[i].mo, players[a_PlayerID].mo))
			continue;
		
		// Player has no object
		if (!players[i].mo)
			continue;
		
		// Initialize spot
		memset(&FakeSpot, 0, sizeof(FakeSpot));
		FakeSpot.type = a_PlayerID + 1;
		
		// Try spawning near them
		for (x = -1; x <= 1; x++)
			for (y = -1; y <= 1; y++)
			{
				// Only in square-ish pattern
				if (/*x != 0 && y != 0 &&*/ x == y)
					continue;
				
				// Setup spot next to player
				FakeSpot.x = (players[i].mo->x >> FRACBITS) + (40 * x);
				FakeSpot.y = (players[i].mo->y >> FRACBITS) + (40 * y);
				
				// Check the area to and around the wanted spot
				if (!GS_CheckSpotArea(
							players[i].mo->x,
							players[i].mo->y,
							((fixed_t)FakeSpot.x) << FRACBITS,
							((fixed_t)FakeSpot.y) << FRACBITS
						))
					continue;
				
				// Check it, if it works return from there
				if (G_CheckSpot(a_PlayerID, &FakeSpot, false))
				{
					P_SpawnPlayer(&FakeSpot);
					return true;
				}
			}
	}
	
	/* Failed */
	return false;
}

/* G_DeathMatchSpawnPlayer() -- Spawns a player at one of the random death match spots called at level load and each respawn */
bool_t G_DeathMatchSpawnPlayer(int playernum)
{
	int i, j, n;
	
	/* Use coop start if no DM spots */
	if (!numdmstarts)
	{
		// GhostlyDeath <December 28, 2012> -- If spawn clustering is enabled
		// and there are no spots, try to cluster at a coop start.
		if (P_XGSVal(PGS_PLSPAWNCLUSTERING))
			if (G_ClusterSpawnPlayer(playernum, false))
				return true;
				
		// If displace spawning is enabled, try that
		if (P_XGSVal(PGS_CODISPLACESPAWN))
			if (G_DisplaceSpawnPlayer(playernum))
				return true;
		
		// Otherwise we just get stuck in our player designated spot (if it exists!)
		if (playerstarts[playernum])
			P_SpawnPlayer(playerstarts[playernum]);
		
		// If no spot exists, find another spot
		else
		{
			for (i = 0; i < MAXPLAYERS; i++)
				if (playerstarts[i])
				{
					playerstarts[i]->type = playernum + 1;
					P_SpawnPlayer(playerstarts[i]);
					playerstarts[i]->type = i + 1;
					break;
				}
		}
		
		return true;
	}
	
	if (P_XGSVal(PGS_COONLYTWENTYDMSPOTS))
		n = 20;
	else
		n = 64;
	
	/* Look in normal DM Starts */
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
	
	/* Spawn Clustering */
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
	
	/* Displace spawn player */
	// This will place them next to an adjacent player
	if (P_XGSVal(PGS_CODISPLACESPAWN))
		if (G_DisplaceSpawnPlayer(playernum))
			return true;

	/* no good spot, so the player will probably get stuck */
	if (P_XGSVal(PGS_COALLOWSTUCKSPAWNS))
	{
		P_SpawnPlayer(playerstarts[playernum]);
		return true;
	}
	
	return false;
}

/* G_CoopSpawnPlayer() -- Spawns player at a coop start */
void G_CoopSpawnPlayer(int playernum)
{
	int i;
	
	/* Try using explicit player spot */
	if (G_CheckSpot(playernum, playerstarts[playernum], false))
	{
		P_SpawnPlayer(playerstarts[playernum]);
		return;
	}
	
	/* If that fails, try another player start */
	for (i = 0; i < MAXPLAYERS; i++)
		if (G_CheckSpot(playernum, playerstarts[i], false))
		{
			playerstarts[i]->type = playernum + 1;	// fake as other player
			P_SpawnPlayer(playerstarts[i]);
			playerstarts[i]->type = i + 1;	// restore
			return;
		}
	
	/* Spawn clustering (extra virtual spots) */
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
	
	/* Displace spawn player */
	// This will place them next to an adjacent player
	if (P_XGSVal(PGS_CODISPLACESPAWN))
		if (G_DisplaceSpawnPlayer(playernum))
			return;
	
	/* Allow getting stuck in other players */
	if (P_XGSVal(PGS_COALLOWSTUCKSPAWNS))
		P_SpawnPlayer(playerstarts[playernum]);
	
	/* Try using DM starts instead */
	else
	{
		int selections;
		
		// GhostlyDeath <December 28, 2012> -- Instead of an I_Error, just spawn
		// at (0, 0) instead. Better than nothing!
		if (!numdmstarts)
			P_SpawnPlayerBackup(playernum);
		
		// Otherwise, use a DM spot
		else
		{
			selections = P_Random() % numdmstarts;
			deathmatchstarts[selections]->type = playernum + 1;
			P_SpawnPlayer(deathmatchstarts[selections]);
		}
	}
}

/* G_TeamSpawnPlayer() -- Spawns player at team start */
// Since Legacy never had real team spawning, use my cluster spawning here
bool_t G_TeamSpawnPlayer(const int a_PlayerNum)
{
	/* Use Spawn Clustering */
	// Try Team starts first
	if (G_ClusterSpawnPlayer(a_PlayerNum, false))
		return true;
	
	// Then try DM starts
	if (G_ClusterSpawnPlayer(a_PlayerNum, true))
		return true;
	
	/* Displace, if those failed */
	if (P_XGSVal(PGS_CODISPLACESPAWN))
		if (G_DisplaceSpawnPlayer(a_PlayerNum))
			return true;
	
	/* If all those happened to fail, try normal DM Spawn */
	return G_DeathMatchSpawnPlayer(a_PlayerNum);
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
		
		// Spawn at team start, if possible
		if (P_GMIsTeam() && P_XGSVal(PGS_CONEWGAMEMODES))
			if (G_TeamSpawnPlayer(playernum))
				return;
		
		// spawn at random spot if in death match
		if (P_GMIsDM())
			if (G_DeathMatchSpawnPlayer(playernum))
				return;
		
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
	
	/* Remove frags from other players */
	// this is so people rejoining do not get free kills! joy!
	for (i = 0; i < MAXPLAYERS; i++)
		players[i].frags[playernum] = 0;
	
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
	pNum = a_Player - &players[0];
	
	/* Clear player junk */
	G_ResetPlayer(a_Player);
	
	/* Clear Totals */
	a_Player->addfrags = 0;
	a_Player->killcount = a_Player->itemcount = a_Player->secretcount = 0;
	a_Player->TotalFrags = a_Player->TotalDeaths = 0;
	
	for (i = 0; i < MAXPLAYERS; i++)
		a_Player->frags[i] = 0;
	
	/* Wipe ports and such */
	a_Player->Port = NULL;
	a_Player->ProfileEx = NULL;
	
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

/* G_ResetPlayer() -- Resets player making them lose all their junk */
void G_ResetPlayer(player_t* const a_Player)
{
	int32_t i;
	
	/* Check */
	if (!a_Player)
		return;
	
	/* Reset things */
	a_Player->playerstate = PST_REBORN;
	a_Player->weaponinfo = wpnlev1info;
	
	// Guns?
	if (a_Player->weaponowned)
		memset(a_Player->weaponowned, 0, sizeof(*a_Player->weaponowned) * NUMWEAPONS);
	
	// Ammo?
	if (a_Player->maxammo)
		memset(a_Player->maxammo, 0, sizeof(*a_Player->maxammo) * NUMAMMO);
		
	if (a_Player->ammo)
		memset(a_Player->ammo, 0, sizeof(*a_Player->ammo) * NUMAMMO);
	
	// Kills and Frags
	a_Player->addfrags = 0;
	a_Player->killcount = a_Player->itemcount = a_Player->secretcount = 0;
	a_Player->TotalFrags = a_Player->TotalDeaths = 0;
	
	for (i = 0; i < MAXPLAYERS; i++)
		a_Player->frags[i] = 0;
		
	/* Allocate */
	// Guns
	if (!a_Player->FavoriteWeapons)
		a_Player->FavoriteWeapons = Z_Malloc(sizeof(*a_Player->FavoriteWeapons) * (NUMWEAPONS + 2), PU_STATIC, NULL);
	if (!a_Player->weaponowned)
		a_Player->weaponowned = Z_Malloc(sizeof(*a_Player->weaponowned) * NUMWEAPONS, PU_STATIC, NULL);
	
	// Ammo
	if (!a_Player->maxammo)
		a_Player->maxammo = Z_Malloc(sizeof(*a_Player->maxammo) * NUMAMMO, PU_STATIC, NULL);
	if (!a_Player->ammo)
		a_Player->ammo = Z_Malloc(sizeof(*a_Player->ammo) * NUMAMMO, PU_STATIC, NULL);
}

//
// G_DoCompleted
//
bool_t secretexit;



/* G_ExitLevel() -- Exits the level */
void G_ExitLevel(const bool_t a_Secret, mobj_t* const a_Activator, const char* const a_Message)
{
	/* Do normal exiting routine */
	if (gamestate == GS_LEVEL)
	{
		secretexit = a_Secret;
		gameaction = ga_completed;
	}
	
	/* Print message to all players, the one who exited the level */
	P_ExitMessage(a_Activator, a_Message);
}

/* G_DoCompleted() -- Called when level is exited */
void G_DoCompleted(void)
{
	int i;
	P_LevelInfoEx_t* NewInfo;
	
	gameaction = ga_nothing;
	
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			G_PlayerFinishLevel(i);	// take away cards and stuff
	
	// GhostlyDeath <May 5, 2012> -- Secret Exit?
	if (secretexit)
	{
		wminfo.next = 1;
		
		for (i = 0; i < MAXPLAYERS; i++)
			players[i].didsecret = true;
	}
	else
		wminfo.next = 0;
	
	// Did secret level?
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
	
	/* Level going to enter, possibly */
	NewInfo = NULL;
	
	// Secret Exit?
	if (wminfo.next && g_CurrentLevelInfo->SecretNext)
		NewInfo = P_FindLevelByNameEx(g_CurrentLevelInfo->SecretNext, NULL);
	
	// Normal Exit?
	if (!NewInfo && g_CurrentLevelInfo->NormalNext)
		NewInfo = P_FindLevelByNameEx(g_CurrentLevelInfo->NormalNext, NULL);
	
	// Set next
	wminfo.NextInfo = NewInfo;
	
	/* Player Junk */
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
}

void G_DoWorldDone(void)
{
	/* Change to new level */
	// Level can be changed to?
	if (wminfo.NextInfo)
	{
		if (!P_ExLoadLevel(wminfo.NextInfo, false))
			gamestate = GS_WAITINGPLAYERS;
	}
	
	// Otherwise it was a failure, try something else
		// Just go back to the waiting for players screen
	else
		gamestate = GS_WAITINGPLAYERS;
	
	/* Done exiting level */
	gameaction = ga_nothing;
}

void G_DoneLevelLoad(void)
{
	// GhostlyDeath <February 8, 2012> -- uint64_t to double not impl on MSVC6
	//CONL_PrintF("Load Level in %f sec\n", (float)(I_GetTime() - demostarttime) / TICRATE);
	framecount = 0;
	demostarttime = I_GetTime();
}

