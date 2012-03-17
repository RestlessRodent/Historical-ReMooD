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

/* GhostlyDeath <July 5, 2008> from m_menu.c */
CV_PossibleValue_t skill_cons_t[] = { {1, "I'm too young to die"}
	, {2, "Hey, not too rough"}
	, {3, "Hurt me plenty"}
	, {4, "Ultra violence"}
	, {5, "Nightmare!"}
	, {0, NULL}
};

CV_PossibleValue_t map_cons_t[] = { {1, "map01"}
	, {2, "map02"}
	, {3, "map03"}
	, {4, "map04"}
	, {5, "map05"}
	, {6, "map06"}
	, {7, "map07"}
	, {8, "map08"}
	, {9, "map09"}
	, {10, "map10"}
	, {11, "map11"}
	, {12, "map12"}
	, {13, "map13"}
	, {14, "map14"}
	, {15, "map15"}
	, {16, "map16"}
	, {17, "map17"}
	, {18, "map18"}
	, {19, "map19"}
	, {20, "map20"}
	, {21, "map21"}
	, {22, "map22"}
	, {23, "map23"}
	, {24, "map24"}
	, {25, "map25"}
	, {26, "map26"}
	, {27, "map27"}
	, {28, "map28"}
	, {29, "map29"}
	, {30, "map30"}
	, {31, "map31"}
	, {32, "map32"}
	, {0, NULL}
};

CV_PossibleValue_t exmy_cons_t[] = { {11, "e1m1"}
	, {12, "e1m2"}
	, {13, "e1m3"}
	, {14, "e1m4"}
	, {15, "e1m5"}
	, {16, "e1m6"}
	, {17, "e1m7"}
	, {18, "e1m8"}
	, {19, "e1m9"}
	, {21, "e2m1"}
	, {22, "e2m2"}
	, {23, "e2m3"}
	, {24, "e2m4"}
	, {25, "e2m5"}
	, {26, "e2m6"}
	, {27, "e2m7"}
	, {28, "e2m8"}
	, {29, "e2m9"}
	, {31, "e3m1"}
	, {32, "e3m2"}
	, {33, "e3m3"}
	, {34, "e3m4"}
	, {35, "e3m5"}
	, {36, "e3m6"}
	, {37, "e3m7"}
	, {38, "e3m8"}
	, {39, "e3m9"}
	, {41, "e4m1"}
	, {42, "e4m2"}
	, {43, "e4m3"}
	, {44, "e4m4"}
	, {45, "e4m5"}
	, {46, "e4m6"}
	, {47, "e4m7"}
	, {48, "e4m8"}
	, {49, "e4m9"}
	, {0, NULL}
};

consvar_t cv_skill = { "skill", "4", CV_HIDEN, skill_cons_t };
consvar_t cv_monsters = { "monsters", "0", CV_HIDEN, CV_YesNo };
consvar_t cv_nextmap = { "nextmap", "1", CV_HIDEN, map_cons_t };

extern CV_PossibleValue_t deathmatch_cons_t[];
consvar_t cv_newdeathmatch = { "newdeathmatch", "3", CV_HIDEN, deathmatch_cons_t };

// added 8-3-98 increse savegame size from 0x2c000 (180kb) to 512*1024
#define SAVEGAMESIZE    (512*1024)
#define SAVESTRINGSIZE  24

bool_t G_CheckDemoStatus(void);
void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum);
void G_InitNew(skill_t skill, char* mapname, bool_t resetplayer);

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
char gamemapname[MAX_WADPATH];	// an external wad filename

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
bool_t playeringame[MAXPLAYERS];
player_t players[MAXPLAYERS];

int consoleplayer[MAXSPLITSCREENPLAYERS];	// player taking events and displaying
int displayplayer[MAXSPLITSCREENPLAYERS];	// view being displayed
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

void ShowMessage_OnChange(void);
void AllowTurbo_OnChange(void);

CV_PossibleValue_t showmessages_cons_t[] = { {0, "Off"}, {1, "On"}, {2, "Not All"}, {0, NULL} };
CV_PossibleValue_t crosshair_cons_t[] = { {0, "Off"}, {1, "Cross"}, {2, "Angle"}, {3, "Point"}, {0, NULL} };

consvar_t cv_crosshair = { "crosshair", "0", CV_SAVE, crosshair_cons_t };

//consvar_t cv_crosshairscale   = {"crosshairscale","0",CV_SAVE,CV_YesNo};
consvar_t cv_autorun = { "autorun", "0", CV_SAVE, CV_OnOff };
consvar_t cv_autorun2 = { "autorun2", "0", CV_SAVE, CV_OnOff };
consvar_t cv_invertmouse = { "invertmouse", "0", CV_SAVE, CV_OnOff };
consvar_t cv_alwaysfreelook = { "alwaysmlook", "0", CV_SAVE, CV_OnOff };
consvar_t cv_invertmouse2 = { "invertmouse2", "0", CV_SAVE, CV_OnOff };
consvar_t cv_alwaysfreelook2 = { "alwaysmlook2", "0", CV_SAVE, CV_OnOff };

consvar_t cv_showmessages = { "showmessages", "1", CV_SAVE | CV_CALL | CV_NOINIT, showmessages_cons_t,
	ShowMessage_OnChange
};
consvar_t cv_disabledemos = { "disabledemos", "0", CV_SAVE, CV_YesNo };
consvar_t cv_allowturbo = { "allowturbo", "0", CV_NETVAR | CV_CALL, CV_YesNo, AllowTurbo_OnChange };
consvar_t cv_mousemove = { "mousemove", "1", CV_SAVE, CV_OnOff };
consvar_t cv_mousemove2 = { "mousemove2", "1", CV_SAVE, CV_OnOff };
consvar_t cv_joystickfreelook = { "joystickfreelook", "0", CV_SAVE, CV_OnOff };

// changed to 2d array 19990220 by Kin
char player_names[MAXPLAYERS][MAXPLAYERNAME];
char team_names[MAXPLAYERS][MAXPLAYERNAME * 2];

mobj_t* bodyque[BODYQUESIZE];
int bodyqueslot;

void* statcopy;					// for statistics driver

void ShowMessage_OnChange(void)
{
	if (!cv_showmessages.value)
		CONL_PrintF("%s\n", MSGOFF);
	else
		CONL_PrintF("%s\n", MSGON);
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
#define KB_LOOKSPEED    (1<<25)
#define MAXPLMOVE       (forwardmove[1])
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS    (6)

static fixed_t forwardmove[2] = { 25, 50 };
static fixed_t sidemove[2] = { 24, 40 };
static fixed_t angleturn[3] = { 640, 1280, 320 };	// + slow turn

// for change this table change also nextweapon func in g_game and P_PlayerThink
uint8_t nextweaponorder[NUMWEAPONS] =
{
	wp_fist, wp_chainsaw, wp_pistol,
	wp_shotgun, wp_supershotgun, wp_chaingun, wp_missile, wp_plasma, wp_bfg
};

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
	fprintf(stderr, "The next gun from %i is %i.\n", g, fw);
	return fw;
}

uint8_t BestWeapon(player_t* player)
{
	int newweapon = FindBestWeapon(player);
	
	if (newweapon == player->readyweapon)
		return 0;
		
	if (newweapon == wp_chainsaw)
		return (BT_CHANGE | BT_EXTRAWEAPON | (wp_fist << BT_WEAPONSHIFT));
		
	if (newweapon == wp_supershotgun)
		return (BT_CHANGE | BT_EXTRAWEAPON | (wp_shotgun << BT_WEAPONSHIFT));
		
	return (BT_CHANGE | (newweapon << BT_WEAPONSHIFT));
}

#define GAMEKEYDOWN(x) (gamekeydown[gamecontrol[player][x][0]] || gamekeydown[gamecontrol[player][x][1]])

void G_BuildTiccmd(ticcmd_t* cmd, int realtics, int player)
{
#define MAXWEAPONSLOTS 64
	int i, j, k, l;
	bool_t strafe;
	int speed;
	int tspeed;
	int forward;
	int side;
	ticcmd_t* base;
	
	int slot;
	weapontype_t newweapon;
	weapontype_t SlotList[MAXWEAPONSLOTS];
	bool_t GunInSlot = false;
	
	//added:14-02-98: these ones used for multiple conditions
	bool_t turnleft, turnright, mouseaiming, analogjoystickmove, gamepadjoystickmove;
	player_t* ply = &players[consoleplayer[player]];
	
	static int turnheld[MAXSPLITSCREENPLAYERS];	// for accelerative turning
	static bool_t keyboard_look;	// true if lookup/down using keyboard
	
	base = I_BaseTiccmd();		// empty, or external driver
	memcpy(cmd, base, sizeof(*cmd));
	
	// a little clumsy, but then the g_input.c became a lot simpler!
	strafe = GAMEKEYDOWN(gc_strafe);
	speed = GAMEKEYDOWN(gc_speed) ^ cv_autorun.value;
	
	turnright = GAMEKEYDOWN(gc_turnright);
	turnleft = GAMEKEYDOWN(gc_turnleft);
	mouseaiming = GAMEKEYDOWN(gc_mouseaiming) ^ cv_alwaysfreelook.value;
	analogjoystickmove = cv_use_joystick.value && !cv_splitscreen.value;
	gamepadjoystickmove = cv_use_joystick.value && !cv_splitscreen.value;
	
	if (gamepadjoystickmove)
	{
		turnright = turnright || (joyxmove > 0);
		turnleft = turnleft || (joyxmove < 0);
	}
	forward = side = 0;
	
	// use two stage accelerative turning
	// on the keyboard and joystick
	if (turnleft || turnright)
		turnheld[player] += realtics;
	else
		turnheld[player] = 0;
		
	if (turnheld[player] < SLOWTURNTICS)
		tspeed = 2;				// slow turn
	else
		tspeed = speed;
		
	// let movement keys cancel each other out
	if (strafe)
	{
		if (turnright)
			side += sidemove[speed];
		if (turnleft)
			side -= sidemove[speed];
			
		if (analogjoystickmove)
		{
			//faB: JOYAXISRANGE is supposed to be 1023 ( divide by 1024 )
			side += ((joyxmove * sidemove[1]) >> 10);
		}
	}
	else
	{
		if (turnright)
			cmd->angleturn -= angleturn[tspeed];
		else if (turnleft)
			cmd->angleturn += angleturn[tspeed];
		if (joyxmove && analogjoystickmove)
		{
			//faB: JOYAXISRANGE should be 1023 ( divide by 1024 )
			cmd->angleturn -= ((joyxmove * angleturn[1]) >> 10);	// ANALOG!
			//CONL_PrintF ("joyxmove %d  angleturn %d\n", joyxmove, cmd->angleturn);
		}
		
	}
	
	//added:07-02-98: forward with key or button
	if (GAMEKEYDOWN(gc_forward) || (joyymove < 0 && gamepadjoystickmove && !cv_joystickfreelook.value))
	{
		forward += forwardmove[speed];
	}
	if (GAMEKEYDOWN(gc_backward) || (joyymove > 0 && gamepadjoystickmove && !cv_joystickfreelook.value))
	{
		forward -= forwardmove[speed];
	}
	
	if (joyymove && analogjoystickmove && !cv_joystickfreelook.value)
		forward -= ((joyymove * forwardmove[1]) >> 10);	// ANALOG!
		
	//added:07-02-98: some people strafe left & right with mouse buttons
	if (GAMEKEYDOWN(gc_straferight))
		side += sidemove[speed];
	if (GAMEKEYDOWN(gc_strafeleft))
		side -= sidemove[speed];
		
	//added:07-02-98: fire with any button/key
	if (GAMEKEYDOWN(gc_fire))
		cmd->buttons |= BT_ATTACK;
		
	//added:07-02-98: use with any button/key
	if (GAMEKEYDOWN(gc_use))
		cmd->buttons |= BT_USE;
		
	//added:22-02-98: jump button
	if (cv_allowjump.value && GAMEKEYDOWN(gc_jump))
		cmd->buttons |= BT_JUMP;
		
	//added:07-02-98: any key / button can trigger a weapon
	// chainsaw overrides
	if (GAMEKEYDOWN(gc_nextweapon))
	{
		// Set switch
		cmd->buttons |= BT_CHANGE;
		cmd->XNewWeapon = NextWeapon(&players[consoleplayer[player]], 1);
	}
	else if (GAMEKEYDOWN(gc_prevweapon))
	{
		// Set switch
		cmd->buttons |= BT_CHANGE;
		cmd->XNewWeapon = NextWeapon(&players[consoleplayer[player]], -1);
	}
	else if (GAMEKEYDOWN(gc_bestweapon))
	{
		cmd->buttons |= BestWeapon(&players[consoleplayer[player]]);
	}
	else
	{
		// Which slot?
		slot = -1;
		
		// Look for keys
		for (i = gc_weapon1; i < gc_weapon1 + 9 - 1; i++)
			if (GAMEKEYDOWN(i))
				slot = (i - gc_weapon1) + 1;
		
		// Hit slot?
		if (slot != -1)
		{
			// Clear flag
			GunInSlot = false;
			l = 0;
		
			// Figure out weapons that belong in this slot
			for (j = 0, i = 0; i < NUMWEAPONS; i++)
				if (P_CanUseWeapon(ply, i))
				{
					// Weapon not in this slot?
					if (ply->weaponinfo[i].SlotNum != slot)
						continue;
				
					// Place in slot list before the highest
					if (j < (MAXWEAPONSLOTS - 1))
					{
						// Just place here
						if (j == 0)
						{
							// Current weapon is in this slot?
							if (ply->readyweapon == i)
							{
								GunInSlot = true;
								l = j;
							}
						
							// Place in last spot
							SlotList[j++] = i;
						}
					
						// Otherwise more work is needed
						else
						{
							// Start from high to low
								// When the order is lower, we know to insert now
							for (k = 0; k < j; k++)
								if (ply->weaponinfo[i].SwitchOrder < ply->weaponinfo[SlotList[k]].SwitchOrder)
								{
									// Current gun may need shifting
									if (!GunInSlot)
									{
										// Current weapon is in this slot?
										if (ply->readyweapon == i)
										{
											GunInSlot = true;
											l = k;
										}
									}
								
									// Possibly shift gun
									else
									{
										// If the current gun is higher then this gun
										// then it will be off by whatever is more
										if (ply->weaponinfo[SlotList[l]].SwitchOrder > ply->weaponinfo[i].SwitchOrder)
											l++;
									}
								
									// move up
									memmove(&SlotList[k + 1], &SlotList[k], sizeof(SlotList[k]) * (MAXWEAPONSLOTS - k - 1));
								
									// Place in slightly upper spot
									SlotList[k] = i;
									j++;
								
									// Don't add it anymore
									break;
								}
						
							// Can't put it anywhere? Goes at end then
							if (k == j)
							{
								// Current weapon is in this slot?
								if (ply->readyweapon == i)
								{
									GunInSlot = true;
									l = k;
								}
							
								// Put
								SlotList[j++] = i;
							}
						}
					}
				}
		
			// No guns in this slot? Then don't switch to anything
			if (j == 0)
				newweapon = ply->readyweapon;
		
			// If the current gun is in this slot, go to the next in the slot
			else if (GunInSlot)		// from [best - worst]
				newweapon = SlotList[((l - 1) + j) % j];
		
			// Otherwise, switch to the best gun there
			else
				// Set it to the highest valued gun
				newweapon = SlotList[j - 1];
		
			// Did it work?
			if (newweapon != ply->readyweapon)
			{
				cmd->buttons |= BT_CHANGE;
				cmd->XNewWeapon = newweapon;
			}
		}
	}
		
	// mouse look stuff (mouse look is not the same as mouse aim)
	if (cv_m_legacymouse.value)
	{
		if (mouseaiming)
		{
			keyboard_look = false;
			
			// looking up/down
			if (cv_invertmouse.value)
				localaiming[player] -= mlooky << 19;
			else
				localaiming[player] += mlooky << 19;
		}
	}
	else
	{
		// X-Axis
		if (cv_m_xaxismode.value == 3)
			localaiming[player] += mousex << 19;
		else if (cv_m_xaxismode.value == 9)
			localaiming[player] -= mousex << 19;
			
		// Y-Axis
		if (cv_m_yaxismode.value == 3)
			localaiming[player] += mousey << 19;
		else if (cv_m_yaxismode.value == 9)
			localaiming[player] -= mousey << 19;
	}
	
	if (cv_use_joystick.value && analogjoystickmove && cv_joystickfreelook.value)
		localaiming[player] += joyymove << 16;
		
	// spring back if not using keyboard neither mouselookin'
	if (!keyboard_look && !cv_joystickfreelook.value && !mouseaiming)
		localaiming[player] = 0;
		
	if (GAMEKEYDOWN(gc_lookup))
	{
		localaiming[player] += KB_LOOKSPEED;
		keyboard_look = true;
	}
	else if (GAMEKEYDOWN(gc_lookdown))
	{
		localaiming[player] -= KB_LOOKSPEED;
		keyboard_look = true;
	}
	else if (GAMEKEYDOWN(gc_centerview))
		localaiming[player] = 0;
		
	//26/02/2000: added by Hurdler: accept no mlook for network games
	if (!cv_allowmlook.value)
		localaiming[player] = 0;
		
	cmd->aiming = G_ClipAimingPitch(&localaiming);
	
	if (cv_m_legacymouse.value)
	{
		if (!mouseaiming && cv_mousemove.value)
			forward += mousey;
			
		if (strafe)
			side += mousex * 2;
		else
			cmd->angleturn -= mousex * 8;
	}
	else
	{
		/* X-Axis */
		// Looking
		if (strafe && cv_m_classicalt.value)
		{
			if (cv_m_xaxismode.value == 1)
				side += mousex;
			else if (cv_m_xaxismode.value == 7)
				side -= mousex;
		}
		else
		{
			if (cv_m_xaxismode.value == 1)
				cmd->angleturn -= mousex * 8;
			else if (cv_m_xaxismode.value == 7)
				cmd->angleturn += mousex * 8;
		}
		
		// Moving (On the X Axis)
		if (cv_m_xaxismode.value == 0)
			side += mousex;
		else if (cv_m_xaxismode.value == 6)
			side -= mousex;
			
		// Moving (On the Y Axis)
		if (cv_m_xaxismode.value == 2)
			forward += mousex;
		else if (cv_m_xaxismode.value == 8)
			forward -= mousex;
			
		/* Y-Axis */
		// Looking
		if (strafe && cv_m_classicalt.value)
		{
			if (cv_m_yaxismode.value == 1)
				side += mousey;
			else if (cv_m_yaxismode.value == 7)
				side -= mousey;
		}
		else
		{
			if (cv_m_yaxismode.value == 1)
				cmd->angleturn -= mousey * 8;
			else if (cv_m_yaxismode.value == 7)
				cmd->angleturn += mousey * 8;
		}
		
		// Moving (On the X Axis)
		if (cv_m_yaxismode.value == 0)
			side += mousey;
		else if (cv_m_yaxismode.value == 6)
			side -= mousey;
			
		// Moving (On the Y Axis)
		if (cv_m_yaxismode.value == 2)
			forward += mousey;
		else if (cv_m_yaxismode.value == 8)
			forward -= mousey;
	}
	
	mousex = mousey = mlooky = 0;
	
	if (forward > MAXPLMOVE)
		forward = MAXPLMOVE;
	else if (forward < -MAXPLMOVE)
		forward = -MAXPLMOVE;
	if (side > MAXPLMOVE)
		side = MAXPLMOVE;
	else if (side < -MAXPLMOVE)
		side = -MAXPLMOVE;
		
	// Heretic Inventory
	if (inventory)
	{
		if (GAMEKEYDOWN(gc_invprev))
		{
			if (ply->st_inventoryTics)
			{
				ply->inv_ptr--;
				if (ply->inv_ptr < 0)
					ply->inv_ptr = 0;
				else
				{
					ply->st_curpos--;
					if (ply->st_curpos < 0)
						ply->st_curpos = 0;
				}
			}
			ply->st_inventoryTics = 5 * TICRATE;
		}
		
		if (GAMEKEYDOWN(gc_invnext))
		{
			if (ply->st_inventoryTics)
			{
				ply->inv_ptr++;
				if (ply->inv_ptr >= ply->inventorySlotNum)
				{
					ply->inv_ptr--;
					if (ply->inv_ptr < 0)
						ply->inv_ptr = 0;
				}
				else
				{
					ply->st_curpos++;
					if (ply->st_curpos > 6)
						ply->st_curpos = 6;
				}
			}
			ply->st_inventoryTics = 5 * TICRATE;
		}
		
		if (GAMEKEYDOWN(gc_invuse))
			cmd->artifact = ply->inventory[ply->st_curpos].type + 1;
	}
	
	cmd->forwardmove += forward;
	cmd->sidemove += side;
#ifdef ABSOLUTEANGLE
	localangle[player] += (cmd->angleturn << 16);
	cmd->angleturn = localangle[player] >> 16;
#endif
	
	//if (gamemode == heretic)
	//{
	if (GAMEKEYDOWN(gc_flydown))
		cmd->angleturn |= BT_FLYDOWN;
	else
		cmd->angleturn &= ~BT_FLYDOWN;
	//}
#undef MAXWEAPONSLOTS
}

static fixed_t originalforwardmove[2] = { 0x19, 0x32 };
static fixed_t originalsidemove[2] = { 0x18, 0x28 };

void AllowTurbo_OnChange(void)
{
	if (!cv_allowturbo.value && netgame)
	{
		// like turbo 100
		forwardmove[0] = originalforwardmove[0];
		forwardmove[1] = originalforwardmove[1];
		sidemove[0] = originalsidemove[0];
		sidemove[1] = originalsidemove[1];
	}
}

//  turbo <10-255>
//
void Command_Turbo_f(void)
{
	int scale = 200;
	
	if (!cv_allowturbo.value && netgame)
	{
		CONL_PrintF("This server don't allow turbo\n");
		return;
	}
	
	if (COM_Argc() != 2)
	{
		CONL_PrintF("turbo <10-255> : set turbo");
		return;
	}
	
	scale = atoi(COM_Argv(1));
	
	if (scale < 10)
		scale = 10;
	if (scale > 255)
		scale = 255;
		
	CONL_PrintF("turbo scale: %i%%\n", scale);
	
	forwardmove[0] = originalforwardmove[0] * scale / 100;
	forwardmove[1] = originalforwardmove[1] * scale / 100;
	sidemove[0] = originalsidemove[0] * scale / 100;
	sidemove[1] = originalsidemove[1] * scale / 100;
}

//
// G_DoLoadLevel
//
void G_DoLoadLevel(bool_t resetplayer)
{
	int i;
	int j = 0;
	char tMap[9];
	
	levelstarttic = gametic;	// for time calculation
	
	if (wipegamestate == GS_LEVEL)
		wipegamestate = -1;		// force a wipe
		
	gamestate = GS_LEVEL;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (resetplayer || (playeringame[i] && players[i].playerstate == PST_DEAD))
			players[i].playerstate = PST_REBORN;
			
		if (playeringame[i])
			j++;
			
		memset(players[i].frags, 0, sizeof(players[i].frags));
		players[i].addfrags = 0;
	}
	
	multiplayer = j > 1;
	
	if (!P_SetupLevel(gameepisode, gamemap, gameskill, gamemapname[0] ? gamemapname : NULL))
	{
		// fail so reset game stuff
		Command_ExitGame_f();
		return;
	}
	//BOT_InitLevelBots ();
	
	displayplayer[0] = consoleplayer[0];	// view the guy you are playing
	if (!cv_splitscreen.value)
		displayplayer[1] = consoleplayer[0];
	else
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
			displayplayer[i] = consoleplayer[i];
			
	gameaction = ga_nothing;
	
	for (i = 0; i < MAXPLAYERS; i++)
		if (players[i].camera.chase)
			P_ResetCamera(&players[i]);
		
	// clear cmd building stuff
	memset(gamekeydown, 0, sizeof(gamekeydown));
	joyxmove = joyymove = 0;
	mousex = mousey = 0;
	
	// clear hud messages remains (usually from game startup)
	HU_ClearFSPics();
	CON_ClearHUD();
	
}

//
// G_Responder
//  Get info needed to make ticcmd_ts for the players.
//
bool_t G_Responder(event_t* ev)
{
	// allow spy mode changes even during the demo
	if (gamestate == GS_LEVEL && ev->type == ev_keydown && ev->data1 == KEY_F12 && (singledemo || !cv_deathmatch.value))
	{
		// spy mode
		do
		{
			displayplayer[0]++;
			if (displayplayer[0] == MAXPLAYERS)
				displayplayer[0] = 0;
		}
		while (!playeringame[displayplayer[0]] && displayplayer[0] != consoleplayer[0]);
		
		//added:16-01-98:change statusbar also if playingback demo
		if (singledemo)
			ST_changeDemoView();
			
		//added:11-04-98: tell who's the view
		CONL_PrintF("Viewpoint : %s\n", player_names[displayplayer[0]]);
		
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
		if (HU_Responder(ev))
			return true;		// chat ate the event
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
	// update keys current state
	G_MapEventsToControls(ev);
	
	switch (ev->type)
	{
		case ev_keydown:
			if (ev->data1 == KEY_PAUSE)
			{
				COM_BufAddText("pause\n");
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

//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker(void)
{
	uint32_t i;
	int buf;
	ticcmd_t* cmd;
	
	// do player reborns if needed
	if (gamestate == GS_LEVEL)
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (playeringame[i])
			{
				if (players[i].playerstate == PST_REBORN)
					G_DoReborn(i);
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
	for (i = 0; i < MAXPLAYERS; i++)
	{
		// BP: i==0 for playback of demos 1.29 now new players is added with xcmd
		if ((playeringame[i] || i == 0) && !dedicated)
		{
			cmd = &players[i].cmd;
			
			if (demoplayback)
				G_ReadDemoTiccmd(cmd, i);
			else
				memcpy(cmd, &netcmds[buf][i], sizeof(ticcmd_t));
				
			if (demorecording)
				G_WriteDemoTiccmd(cmd, i);
				
			// check for turbo cheats
			if (cmd->forwardmove > TURBOTHRESHOLD && !(gametic % (32)) && ((gametic / (32)) & 3) == i)
			{
				static char turbomessage[80];
				
				sprintf(turbomessage, "%s is turbo!", player_names[i]);
				players[consoleplayer[0]].message = turbomessage;
			}
		}
	}
	
	// do main actions
	switch (gamestate)
	{
		case GS_LEVEL:
			P_Ticker();			// tic the game
			ST_Ticker();
			AM_Ticker();
			HU_Ticker();
			break;
			
		case GS_INTERMISSION:
			WI_Ticker();
			break;
			
		case GS_FINALE:
			F_Ticker();
			break;
			
		case GS_DEMOSCREEN:
			D_PageTicker();
			break;
			
		case GS_WAITINGPLAYERS:
		case GS_DEDICATEDSERVER:
		case GS_NULL:
			// do nothing
			break;
	}
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer
// Called at the start.
// Called by the game initialization functions.
//

/* BP:UNUSED !
void G_InitPlayer (int player)
{
    player_t*   p;

    // set up the saved info
    p = &players[player];

    // clear everything else to defaults
    G_PlayerReborn (player);
}
*/

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

void VerifFavoritWeapon(player_t* player);

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
	ProfileInfo_t* prof;
	
	//from Boris
	int skincolor;
	char favoritweapon[NUMWEAPONS];
	bool_t originalweaponswitch;
	bool_t autoaim;
	int skin;					//Fab: keep same skin
	
	prof = players[player].profile;
	memcpy(frags, players[player].frags, sizeof(frags));
	addfrags = players[player].addfrags;
	killcount = players[player].killcount;
	itemcount = players[player].itemcount;
	secretcount = players[player].secretcount;
	
	//from Boris
	skincolor = players[player].skincolor;
	originalweaponswitch = players[player].originalweaponswitch;
	memcpy(favoritweapon, players[player].favoritweapon, NUMWEAPONS);
	autoaim = players[player].autoaim_toggle;
	skin = players[player].skin;
	
	p = &players[player];
	memset(p, 0, sizeof(*p));
	
	memcpy(players[player].frags, frags, sizeof(players[player].frags));
	players[player].addfrags = addfrags;
	players[player].killcount = killcount;
	players[player].itemcount = itemcount;
	players[player].secretcount = secretcount;
	
	// save player config truth reborn
	players[player].skincolor = skincolor;
	players[player].originalweaponswitch = originalweaponswitch;
	memcpy(players[player].favoritweapon, favoritweapon, NUMWEAPONS);
	players[player].autoaim_toggle = autoaim;
	players[player].skin = skin;
	
	p->usedown = p->attackdown = true;	// don't do anything immediately
	p->playerstate = PST_LIVE;
	p->health = initial_health;
	
	p->weaponinfo = wpnlev1info;
	//p->weaponinfo = doomweaponinfo;
	p->readyweapon = p->pendingweapon = wp_pistol;
	p->weaponowned[wp_fist] = true;
	p->weaponowned[wp_pistol] = true;
	p->ammo[am_clip] = initial_bullets;
	
	p->profile = prof;
	
	// Boris stuff
	if (!p->originalweaponswitch)
		VerifFavoritWeapon(p);
	//eof Boris
	
	for (i = 0; i < NUMAMMO; i++)
		p->maxammo[i] = ammoinfo[i].MaxAmmo;
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
bool_t G_CheckSpot(int playernum, mapthing_t* mthing)
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
		
	if (!players[playernum].mo)
	{
		// first spawn of level, before corpses
		for (i = 0; i < playernum; i++)
			// added 15-1-98 check if player is in game (mistake from id)
			if (playeringame[i] && players[i].mo->x == mthing->x << FRACBITS && players[i].mo->y == mthing->y << FRACBITS)
				return false;
		return true;
	}
	
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;
	ss = R_PointInSubsector(x, y);
	
	// check for respawn in team-sector
	if (ss->sector->teamstartsec)
	{
		if (cv_teamplay.value == 1)
		{
			// color
			if (players[playernum].skincolor != (ss->sector->teamstartsec - 1))	// -1 because wanted to know when it is set
				return false;
		}
		else if (cv_teamplay.value == 2)
		{
			// skins
			if (players[playernum].skin != (ss->sector->teamstartsec - 1))	// -1 because wanted to know when it is set
				return false;
		}
	}
	
	if (!P_CheckPosition(players[playernum].mo, x, y))
		return false;
		
	// flush an old corpse if needed
	if (bodyqueslot >= BODYQUESIZE)
		P_RemoveMobj(bodyque[bodyqueslot % BODYQUESIZE]);
	bodyque[bodyqueslot % BODYQUESIZE] = players[playernum].mo;
	bodyqueslot++;
	
	// spawn a teleport fog
	// TODO: Vanilla comp: an = (ANG45 * (mthing->angle / 45)) >> ANGLETOFINESHIFT;
	an = (ANG45 * (mthing->angle / 45));
	an >>= ANGLETOFINESHIFT;
	
	mo = P_SpawnMobj(x + 20 * finecosine[an], y + 20 * finesine[an], ss->sector->floorheight, INFO_GetTypeByName("TeleportFog"));
	
	//added:16-01-98:consoleplayer -> displayplayer (hear snds from viewpt)
	// removed 9-12-98: why not ????
	if (players[displayplayer[0]].viewz != 1)
		S_StartSound(mo, sfx_telept);	// don't start sound on first frame
		
	return true;
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
	
	if (demoversion < 123)
		n = 20;
	else
		n = 64;
		
	for (j = 0; j < n; j++)
	{
		i = P_Random() % numdmstarts;
		if (G_CheckSpot(playernum, deathmatchstarts[i]))
		{
			deathmatchstarts[i]->type = playernum + 1;
			P_SpawnPlayer(deathmatchstarts[i]);
			return true;
		}
	}
	
	if (demoversion < 113)
	{
	
		// no good spot, so the player will probably get stuck
		P_SpawnPlayer(playerstarts[playernum]);
		return true;
	}
	return false;
}

void G_CoopSpawnPlayer(int playernum)
{
	int i;
	
	// no deathmatch use the spot
	if (G_CheckSpot(playernum, playerstarts[playernum]))
	{
		P_SpawnPlayer(playerstarts[playernum]);
		return;
	}
	// try to spawn at one of the other players spots
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (G_CheckSpot(playernum, playerstarts[i]))
		{
			playerstarts[i]->type = playernum + 1;	// fake as other player
			P_SpawnPlayer(playerstarts[i]);
			playerstarts[i]->type = i + 1;	// restore
			return;
		}
		// he's going to be inside something.  Too bad.
	}
	
	if (demoversion < 113 || (demoversion >= 113 && localgame))
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
	
	// boris comment : this test is like 'single player game'
	//                 all this kind of hiden variable must be removed
#if 0
	if (!multiplayer && !cv_deathmatch.value)
	{
		// reload the level from scratch
		G_DoLoadLevel(true);
	}
	else
#endif
	{
		// respawn at the start
		
		// first dissasociate the corpse
		if (player->mo)
		{
			player->mo->player = NULL;
			player->mo->flags2 &= ~MF2_DONTDRAW;
		}
		// spawn at random spot if in death match
		if (cv_deathmatch.value)
		{
			if (G_DeathMatchSpawnPlayer(playernum))
				return;
		}
		
		G_CoopSpawnPlayer(playernum);
	}
}

void G_AddPlayer(int playernum)
{
	player_t* p = &players[playernum];
	
	p->playerstate = PST_REBORN;
	p->weaponinfo = wpnlev1info;
}

// DOOM Par Times
static const int pars[4][10] =
{
	{0},
	{0, 30, 75, 120, 90, 165, 180, 180, 30, 165},
	{0, 90, 90, 90, 120, 90, 360, 240, 30, 170},
	{0, 90, 45, 90, 150, 90, 90, 165, 30, 135}
};

// DOOM II Par Times
static const int cpars[32] =
{
	30, 90, 120, 120, 90, 150, 120, 120, 270, 90,	//  1-10
	210, 150, 150, 150, 210, 150, 420, 150, 210, 150,	// 11-20
	240, 150, 180, 150, 150, 300, 330, 420, 300, 180,	// 21-30
	120, 30						// 31-32
};

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
	// IF NO WOLF3D LEVELS, NO SECRET EXIT!
	if ((gamemode == commercial) && (W_CheckNumForName("map31") < 0))
		secretexit = false;
	else
		secretexit = true;
	gameaction = ga_completed;
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
		
	if (gamemode != commercial)
		switch (gamemap)
		{
			case 8:
				//BP add comment : no intermistion screen
				if (cv_deathmatch.value)
					wminfo.next = 0;
				else
				{
					// also for heretic
					// disconnect from network
					F_StartFinale();
					return;
				}
			case 9:
				for (i = 0; i < MAXPLAYERS; i++)
					players[i].didsecret = true;
				break;
		}
		
	if (!dedicated)
		wminfo.didsecret = players[consoleplayer[0]].didsecret;
	wminfo.epsd = gameepisode - 1;
	wminfo.last = gamemap - 1;
	
	// go to next level
	// wminfo.next is 0 biased, unlike gamemap
	wminfo.next = gamemap;
	
	// overwrite next level in some cases
	if (gamemode == commercial)
	{
		if (secretexit)
			switch (gamemap)
			{
				case 15:
					wminfo.next = 30;
					break;
				case 31:
					wminfo.next = 31;
					break;
				default:
					wminfo.next = 15;
					break;
			}
		else
			switch (gamemap)
			{
				case 31:
				case 32:
					wminfo.next = 15;
					break;
				default:
					wminfo.next = gamemap;
			}
	}
	else
	{
		if (secretexit)
			wminfo.next = 8;	// go to secret level
		else if (gamemap == 9)
		{
			// returning from secret level
			switch (gameepisode)
			{
				case 1:
					wminfo.next = 3;
					break;
				case 2:
					wminfo.next = 5;
					break;
				case 3:
					wminfo.next = 6;
					break;
				case 4:
					wminfo.next = 2;
					break;
				default:
					wminfo.next = 0;
					break;
			}
		}
		else if (gamemap == 8)
			wminfo.next = 0;	// wrape around in deathmatch
	}
	
	wminfo.maxkills = totalkills;
	wminfo.maxitems = totalitems;
	wminfo.maxsecret = totalsecret;
	wminfo.maxfrags = 0;
	if (info_partime != -1)
		wminfo.partime = TICRATE * info_partime;
	else if (gamemode == commercial)
		wminfo.partime = TICRATE * cpars[gamemap - 1];
	else
		wminfo.partime = TICRATE * pars[gameepisode][gamemap];
	wminfo.pnum = consoleplayer[0];
	
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
		players[consoleplayer[0]].didsecret = true;
		
	if (gamemode == commercial)
	{
		if (cv_deathmatch.value == 0)
		{
			switch (gamemap)
			{
				case 15:
				case 31:
					if (!secretexit)
						break;
				case 6:
				case 11:
				case 20:
				case 30:
					gameaction = ga_nothing;
					F_StartFinale();
					break;
			}
		}
		else if (gamemap == 30)
			wminfo.next = 0;	// wrap around in deathmatch
	}
}

void G_DoWorldDone(void)
{
	if (demoversion < 129)
	{
		gamemap = wminfo.next + 1;
		G_DoLoadLevel(true);
	}
	else
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
		
	gameaction = ga_nothing;
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
void G_LoadGame(int slot)
{
	COM_BufAddText(va("load %d\n", slot));
}

#define VERSIONSIZE             16

void G_DoLoadGame(int slot)
{
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
	displayplayer[0] = consoleplayer[0];
	
	// done
	Z_Free(savebuffer);
	
	multiplayer = playeringame[1];
	if (playeringame[1] && !netgame)
		CV_SetValue(&cv_splitscreen, 1);
		
	if (setsizeneeded)
		R_ExecuteSetViewSize();
		
	// draw the pattern into the back screen
	R_FillBackScreen();
	CON_ToggleOff();
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 uint8_t text string
//
void G_SaveGame(int slot, char* description)
{
	if (server)
		COM_BufAddText(va("save %d \"%s\"\n", slot, description));
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
	
	players[consoleplayer[0]].message = GGSAVED;
	
	// draw the pattern into the back screen
	R_FillBackScreen();
#endif
	
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
}

//
// G_InitNew
//  Can be called by the startup code or the menu task,
//  consoleplayer, displayplayer, playeringame[] should be set.
//
// Boris comment : single player start game
void G_DeferedInitNew(skill_t skill, char* mapname, int StartSplitScreenGame)
{
	int i;
	
	G_Downgrade(VERSION);
	paused = false;
	
	// NULL profiles
	for (i = 0; i < MAXPLAYERS; i++)
		players[i].profile = NULL;
		
	// Enable End Game option
	MainDef.menuitems[1].status &= ~IT_DISABLED2;
	
	CV_Set(&cv_splitscreen, va("%d", StartSplitScreenGame));
	
	COM_BufAddText(va("map \"%s\" -skill %d -monsters 1\n", mapname, skill + 1));
}

//
// This is the map command interpretation something like Command_Map_f
//
// called at : map cmd execution, doloadgame, doplaydemo
void G_InitNew(skill_t skill, char* mapname, bool_t resetplayer)
{
	//added:27-02-98: disable selected features for compatibility with
	//                older demos, plus reset new features as default
	if (!G_Downgrade(demoversion))
	{
		CONL_PrintF("Cannot Downgrade engine\n");
		D_StartTitle();
		return;
	}
	
	if (paused)
	{
		paused = false;
		S_ResumeMusic();
	}
	
	if (skill > sk_nightmare)
		skill = sk_nightmare;
		
	M_ClearRandom();
	
	if (skill == sk_nightmare)
	{
		CV_SetValue(&cv_respawnmonsters, 1);
		CV_SetValue(&cv_fastmonsters, 1);
	}
	// for internal maps only
	if (FIL_CheckExtension(mapname))
	{
		// external map file
		strncpy(gamemapname, mapname, MAX_WADPATH);
		gameepisode = 1;
		gamemap = 1;
	}
	else
	{
		// internal game map
		// well this  check is useless because it is done before (d_netcmd.c::command_map_f)
		// but in case of for demos....
		if (W_CheckNumForName(mapname) == -1)
		{
			CONL_PrintF("\2Internal game map '%s' not found\n" "(use .wad extension for external maps)\n", mapname);
			Command_ExitGame_f();
			return;
		}
		
		gamemapname[0] = 0;		// means not an external wad file
		if (gamemode == commercial)	//doom2
		{
			gamemap = atoi(mapname + 3);	// get xx out of MAPxx
			gameepisode = 1;
		}
		else
		{
			gamemap = mapname[3] - '0';	// ExMy
			gameepisode = mapname[1] - '0';
		}
	}
	
	gameskill = skill;
	playerdeadview = false;
	automapactive = false;
	automapoverlay = false;
	
	G_DoLoadLevel(resetplayer);
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

//
// DEMO RECORDING
//

#define ZT_FWD          0x01
#define ZT_SIDE         0x02
#define ZT_ANGLE        0x04
#define ZT_BUTTONS      0x08
#define ZT_AIMING       0x10
#define ZT_CHAT         0x20	// no more used
#define ZT_EXTRADATA    0x40
#define DEMOMARKER      0x80	// demoend

ticcmd_t oldcmd[MAXPLAYERS];

void G_ReadDemoTiccmd(ticcmd_t* cmd, int playernum)
{
#if 0
	if (*demo_p == DEMOMARKER)
	{
		// end of demo data stream
		G_CheckDemoStatus();
		return;
	}
	if (demoversion < 112)
	{
		cmd->forwardmove = READCHAR(demo_p);
		cmd->sidemove = READCHAR(demo_p);
		cmd->angleturn = READBYTE(demo_p) << 8;
		cmd->buttons = READBYTE(demo_p);
		cmd->aiming = 0;
	}
	else
	{
		char ziptic = *demo_p++;
		
		if (ziptic & ZT_FWD)
			oldcmd[playernum].forwardmove = READCHAR(demo_p);
		if (ziptic & ZT_SIDE)
			oldcmd[playernum].sidemove = READCHAR(demo_p);
		if (ziptic & ZT_ANGLE)
		{
			if (demoversion < 125)
				oldcmd[playernum].angleturn = READBYTE(demo_p) << 8;
			else
				oldcmd[playernum].angleturn = READSHORT(demo_p);
		}
		if (ziptic & ZT_BUTTONS)
			oldcmd[playernum].buttons = READBYTE(demo_p);
		if (ziptic & ZT_AIMING)
		{
			if (demoversion < 128)
				oldcmd[playernum].aiming = READCHAR(demo_p);
			else
				oldcmd[playernum].aiming = READSHORT(demo_p);
		}
		if (ziptic & ZT_CHAT)
			demo_p++;
			
		memcpy(cmd, &(oldcmd[playernum]), sizeof(ticcmd_t));
	}
#else
	G_CheckDemoStatus();
	return;
#endif
}

void G_WriteDemoTiccmd(ticcmd_t* cmd, int playernum)
{
}

//
// G_RecordDemo
//
void G_RecordDemo(char* name)
{
	int i;
	int maxsize;
	
	RDF_DEMO_PrepareRecording(name);
	
	demorecording = true;
}

/* G_BeginRecording() -- Records an RDF Demo */
void G_BeginRecording(void)
{
	RDF_DEMO_StartRecording();
}

//
// G_PlayDemo
//

void G_DeferedPlayDemo(char* name)
{
	COM_BufAddText("playdemo \"");
	COM_BufAddText(name);
	COM_BufAddText("\"\n");
}

//
//  Start a demo from a .LMP file or from a wad resource (eg: DEMO1)
//
void G_DoPlayDemo(char* defdemoname)
{
	skill_t skill;
	int i, j, episode, map;
	
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
	displayplayer[0] = consoleplayer[0] = *demo_p++;
	
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
		
	if (cv_splitscreen.value)
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		{
			displayplayer[i] = i;
			consoleplayer[i] = i;
		}
		
	demoplayback = true;
	M_LockGameCVARS();
	DC_SetMenuGameOptions(1);
	DC_SetDemoOptions(demoversion);
#endif
}

//
// G_TimeDemo
//             NOTE: name is a full filename for external demos
//
static int restorecv_vidwait;

void G_TimeDemo(char* name)
{
	int i;
	
	if (cv_splitscreen.value)
		for (i = 0; i < MAXSPLITSCREENPLAYERS; i++)
		{
			displayplayer[i] = i;
			consoleplayer[i] = i;
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
