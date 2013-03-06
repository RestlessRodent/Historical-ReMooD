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
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)

#include "doomdef.h"
#include "g_game.h"
#include "m_cheat.h"
#include "screen.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "m_random.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "i_video.h"
#include "v_video.h"
#include "z_zone.h"
#include "hu_stuff.h"
#include "d_main.h"
#include "p_demcmp.h"
#include "vhw_wrap.h"
#include "p_info.h"
#include "wi_stuff.h"
#include "m_menu.h"

//protos
void ST_createWidgets(void);

extern fixed_t waterheight;

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            (1 * VPALSMOOTHCOUNT)
#define STARTBONUSPALS          (9 * VPALSMOOTHCOUNT)
#define NUMREDPALS              (8 * VPALSMOOTHCOUNT)
#define NUMBONUSPALS            (4 * VPALSMOOTHCOUNT)
// Radiation suit, green shift.
#define RADIATIONPAL            (13 * VPALSMOOTHCOUNT)

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY              96

// For Responder
#define ST_TOGGLECHAT           KEY_ENTER

// Location of status bar
//added:08-01-98:status bar position changes according to resolution.
#define ST_FX                     143

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH         (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

#define ST_FACESX               143
#define ST_FACESY               (ST_Y+0)

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH            3
#define ST_AMMOX                44
#define ST_AMMOY                (ST_Y+3)

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
#define ST_HEALTHX              90
#define ST_HEALTHY              (ST_Y+3)

// Weapon pos.
#define ST_ARMSX                111
#define ST_ARMSY                (ST_Y+4)
#define ST_ARMSBGX              104
#define ST_ARMSBGY              (ST_Y)
#define ST_ARMSXSPACE           12
#define ST_ARMSYSPACE           10

// Frags pos.
#define ST_FRAGSX               138
#define ST_FRAGSY               (ST_Y+3)
#define ST_FRAGSWIDTH           2

// ARMOR number pos.
#define ST_ARMORWIDTH           3
#define ST_ARMORX               221
#define ST_ARMORY               (ST_Y+3)

// Key icon positions.
#define ST_KEY0WIDTH            8
#define ST_KEY0HEIGHT           5
#define ST_KEY0X                239
#define ST_KEY0Y                (ST_Y+3)
#define ST_KEY1WIDTH            ST_KEY0WIDTH
#define ST_KEY1X                239
#define ST_KEY1Y                (ST_Y+13)
#define ST_KEY2WIDTH            ST_KEY0WIDTH
#define ST_KEY2X                239
#define ST_KEY2Y                (ST_Y+23)

// Ammunition counter.
#define ST_AMMO0WIDTH           3
#define ST_AMMO0HEIGHT          6
#define ST_AMMO0X               288
#define ST_AMMO0Y               (ST_Y+5)
#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
#define ST_AMMO1X               288
#define ST_AMMO1Y               (ST_Y+11)
#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
#define ST_AMMO2X               288
#define ST_AMMO2Y               (ST_Y+23)
#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
#define ST_AMMO3X               288
#define ST_AMMO3Y               (ST_Y+17)

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
#define ST_MAXAMMO0HEIGHT       5
#define ST_MAXAMMO0X            314
#define ST_MAXAMMO0Y            (ST_Y+5)
#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X            314
#define ST_MAXAMMO1Y            (ST_Y+11)
#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X            314
#define ST_MAXAMMO2Y            (ST_Y+23)
#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X            314
#define ST_MAXAMMO3Y            (ST_Y+17)

//faB: unused stuff from the Doom alpha version ?
// pistol
//#define ST_WEAPON0X           110
//#define ST_WEAPON0Y           (ST_Y+4)
// shotgun
//#define ST_WEAPON1X           122
//#define ST_WEAPON1Y           (ST_Y+4)
// chain gun
//#define ST_WEAPON2X           134
//#define ST_WEAPON2Y           (ST_Y+4)
// missile launcher
//#define ST_WEAPON3X           110
//#define ST_WEAPON3Y           (ST_Y+13)
// plasma gun
//#define ST_WEAPON4X           122
//#define ST_WEAPON4Y           (ST_Y+13)
// bfg
//#define ST_WEAPON5X           134
//#define ST_WEAPON5Y           (ST_Y+13)

// WPNS title
//#define ST_WPNSX              109
//#define ST_WPNSY              (ST_Y+23)

// DETH title
//#define ST_DETHX              109
//#define ST_DETHY              (ST_Y+23)

//Incoming messages window location
// #define ST_MSGTEXTX     (viewwindowx)
// #define ST_MSGTEXTY     (viewwindowy+viewheight-18)
//#define ST_MSGTEXTX             0
//#define ST_MSGTEXTY             0     //added:08-01-98:unused
// Dimensions given in characters.
#define ST_MSGWIDTH             52
// Or shall I say, in lines?
#define ST_MSGHEIGHT            1

#define ST_OUTTEXTX             0
#define ST_OUTTEXTY             6

// Width, in characters again.
#define ST_OUTWIDTH             52
// Height, in lines.
#define ST_OUTHEIGHT            1

#define ST_MAPWIDTH     \
    (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

//added:24-01-98:unused ?
//#define ST_MAPTITLEX  (vid.width - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY            0
#define ST_MAPHEIGHT            1

//added:02-02-98: set true if widgets coords need to be recalculated
bool_t st_recalc;

// main player in game
//Hurdler: no not static!
player_t* plyr;

// ST_Start() has just been called
bool_t st_firsttime;

// used to execute ST_Init() only once
static int veryfirsttime = 1;

// used for timing
static unsigned int st_clock;

int stbarheight = ST_HEIGHT;
int ST_Y = BASEVIDHEIGHT - ST_HEIGHT;
int st_x = 0;
float st_scalex, st_scaley;

// ------------------------------------------
//             status bar overlay
// ------------------------------------------

// icons for overlay
static int sbohealth;
static int sbofrags;
static int sboarmor;

void ST_TransSTChange(void)
{
	R_ExecuteSetViewSize();
}

void ST_ExternrefreshBackground(void)
{
}

bool_t ST_SameTeam(player_t* a, player_t* b)
{
	// New Modes
	if (P_XGSVal(PGS_CONEWGAMEMODES))
	{
		// Team Play?
		if (P_GMIsTeam())
		{
			// One player is neutral?
			if (a->VTeamColor <= -1 || b->VTeamColor <= -1)
				return false;
			
			// Same VTeam?
			if (a->VTeamColor == b->VTeamColor)
				return true;
			
			// Not same team
			return false;
		}
		
		// Otherwise not
		else
			return false;
	}
	
	// Old Modes
	else
		switch (P_XGSVal(PGS_GAMETEAMPLAY))
		{
			case 0:
				return false;
			case 1:
				return (a->skincolor == b->skincolor);
			case 2:
				return (a->skin == b->skin);
		}
	
	return false;
}

// count the frags of the playernum player
//Fab: made as a tiny routine so ST_overlayDrawer() can use it
//Boris: rename ST_countFrags in to ST_PlayerFrags for use anytime
//       when we need the frags
int ST_PlayerFrags(int playernum)
{
	int32_t i, frags;
	
	frags = players[playernum].addfrags;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if ((!P_GMIsTeam() && i != playernum) || (P_GMIsTeam() && !ST_SameTeam(&players[i], &players[playernum])))
			frags += players[playernum].frags[i];
		else
			frags -= players[playernum].frags[i];
	}
	
	return frags;
}

static bool_t st_stopped = true;

void ST_Ticker(void)
{
}

static int st_palette = 0;

/* ST_doPaletteStuff() -- Changes the current palette */
// GhostlyDeath <July 30, 2011> -- Redone for smoothing
void ST_doPaletteStuff(void)
{
	int ChosePal = 0;
	int BaseDam = 0, BzFade;
	
	/* Berserker fade */
	BaseDam = plyr->damagecount;
	
	if (plyr->powers[pw_strength])
	{
		BzFade = 12 - (plyr->powers[pw_strength] >> 6);
		
		if (BzFade > BaseDam)
			BaseDam = BzFade;
	}
	
	/* Player is hurt? */
	if (BaseDam)
	{
		// Division is number of palettes
		ChosePal = FixedMul(NUMREDPALS << FRACBITS, FixedDiv((BaseDam) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
		ChosePal++;				// +7
		//ChosePal = (plyr->damagecount * (10000 / NUMREDPALS) ) / 100;
		
		// Don't exceed
		if (ChosePal >= NUMREDPALS)
			ChosePal = NUMREDPALS - 1;
			
		// Offset
		ChosePal += STARTREDPALS;
	}
	
	/* Player got an item */
	else if (plyr->bonuscount)
	{
		// Division is number of palettes
		ChosePal = FixedMul(NUMBONUSPALS << FRACBITS, FixedDiv((plyr->bonuscount) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
		ChosePal++;				// +7
		
		// Don't exceed
		if (ChosePal >= NUMBONUSPALS)
			ChosePal = NUMBONUSPALS - 1;
			
		// Offset
		ChosePal += STARTBONUSPALS;
	}
	
	/* Player has radiation suit */
	else if (plyr->powers[pw_ironfeet] > 4 * 32 || plyr->powers[pw_ironfeet] & 8)
	{
		ChosePal = RADIATIONPAL;
	}
	
	/* Set palette */
	if ((g_SplitScreen <= 0))
		V_SetPalette(ChosePal);
}

static void ST_diffDraw(void)
{
}

void ST_Invalidate(void)
{
	st_firsttime = true;
}

void ST_overlayDrawer(void);

void ST_Drawer(bool_t refresh)
{
}

static void ST_loadGraphics(void)
{
}

// made separate so that skins code can reload custom face graphics
void ST_loadFaceGraphics(char* facestr)
{
}

static void ST_loadData(void)
{
}

void ST_unloadGraphics(void)
{
}

// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphics(void)
{
}

void ST_unloadData(void)
{
}

void ST_initData(void)
{
}

static void ST_Stop(void)
{
}

void ST_Start(void)
{
}

//
//  Initializes the status bar,
//  sets the defaults border patch for the window borders.
//

//faB: used by Glide mode, holds lumpnum of flat used to fill space around the viewwindow
int st_borderpatchnum;

void ST_Init(void)
{
}

//added:16-01-98: change the status bar too, when pressing F12 while viewing
//                 a demo.
void ST_changeDemoView(void)
{
}

/*****************************************************************************/

/*** PRIVATE FUNCTIONS ***/

/* STS_SBX() -- Status Bar X */
static int32_t STS_SBX(D_ProfileEx_t* const a_Profile, const int32_t a_Coord, int32_t a_W, const int32_t a_H)
{
	int c = a_Coord;
	
	// a_W    1
	// --- * --- = ???
	//  1    320
	return FixedMul(c << FRACBITS, FixedMul(204, a_W << FRACBITS)) >> FRACBITS;
}

/* STS_SBY() -- Status Bar Y */
static int32_t STS_SBY(D_ProfileEx_t* const a_Profile, const int32_t a_Coord, int32_t a_W, const int32_t a_H)
{
	int c = a_Coord;
	
	// a_H    1
	// --- * --- = ???
	//  1    200
	return FixedMul(c << FRACBITS, FixedMul(327, a_H << FRACBITS)) >> FRACBITS;
}

/* STS_DrawPlayerMap() -- Draws player automap */
static void STS_DrawPlayerMap(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP)
{
	D_ProfileEx_t* Profile;
	
	/* Get profile of player */
	Profile = a_ConsoleP->ProfileEx;
	
	/* Draw something */
	VHW_HUDDrawLine(VHWRGB(255,0,0), 10, 10, 10, 30);	// H
	VHW_HUDDrawLine(VHWRGB(255,0,0), 30, 10, 10, 30);	// H
	VHW_HUDDrawLine(VHWRGB(255,0,0), 10, 20, 30, 20);	// H
	
	/* Current Level Name */
	V_DrawStringA(VFONT_SMALL, 0, P_LevelNameEx(), a_X + STS_SBX(Profile, 20, a_W, a_H), a_Y + (a_H - V_FontHeight(VFONT_SMALL)));
}

extern bool_t g_NetBoardDown;

/* STS_DrawPlayerBarEx() -- Draws a player's status bar, and a few other things */
static void STS_DrawPlayerBarEx(const size_t a_PID, const int32_t a_X, const int32_t a_Y, const int32_t a_W, const int32_t a_H, player_t* const a_ConsoleP, player_t* const a_DisplayP)
{
#define BUFSIZE 32
	char Buf[BUFSIZE];
	player_t* ConsoleP, *DisplayP;
	D_ProfileEx_t* Profile;
	V_Image_t* vi;
	VideoFont_t Font;
	PI_wepid_t ReadyWeapon;
	PI_ammoid_t AmmoType;
	bool_t BigLetters, IsMonster;
	D_XPlayer_t* XPlay;
	bool_t IsFake, OK;
	uint32_t i, j, k;
	PI_key_t* DrawKey;
	int32_t Right;
	
	/* Init */
	// Font to use
	if (a_W < 320)
	{
		BigLetters = false;
		Font = VFONT_SMALL;
	}
	else
	{
		BigLetters = true;
		
		if (g_CoreGame == CG_HERETIC)
			Font = VFONT_LARGE_HERETIC;
		else
			Font = VFONT_STATUSBARLARGE;
	}
	
	/* Get players to draw for */
	ConsoleP = a_ConsoleP;
	DisplayP = a_DisplayP;
	
	// Fake player?
	IsFake = false;
	if (DisplayP == P_SpecGet(a_PID))
		IsFake = true;
	
	// Net player
	XPlay = ConsoleP->XPlayer;
	
	/* Get profile of player */
	Profile = ConsoleP->ProfileEx;
	
	/* Obtain some info */
	if (!IsFake)
	{
		ReadyWeapon = DisplayP->readyweapon;
		AmmoType = NUMAMMO;
		if (DisplayP->weaponinfo)
			AmmoType = DisplayP->weaponinfo[DisplayP->readyweapon]->ammo;
	}
	
	/* Monster? */
	IsMonster = false;
	if (!IsFake)
		if (DisplayP->mo && ((DisplayP->mo->flags & MF_COUNTKILL) || !(DisplayP->mo->RXFlags[0] & MFREXA_ISPLAYEROBJECT)))
			IsMonster = true;
	
	/* Draw the automap */
	if (g_Splits[a_PID].AutomapActive)
	{
		// Draw black box?
		if (!g_Splits[a_PID].OverlayMap)
			VHW_HUDDrawBox(0, 0, 0, 0, a_X, a_Y, a_X + a_W, a_X + a_H);
		
		// Used subroutine
		STS_DrawPlayerMap(a_PID, a_X, a_Y, a_W, a_H, a_ConsoleP, a_DisplayP);
	}
	
	/* Which status bar type to draw? */
	// Overlay
	if (true && !IsFake)
	{
		//// HEALTH
		// Draw Health Icon
		vi = V_ImageFindA((IsMonster ? "sbohealg" : "sbohealt"), VCP_DOOM);
		if (vi)
			V_ImageDraw(
					0, vi,
					a_X + STS_SBX(Profile, 8, a_W, a_H),
					a_Y + STS_SBY(Profile, 192, a_W, a_H) - 16,
					NULL
				);
		
		// Draw Health Text
		if (IsMonster)
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->mo->health);
		else
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->health);
		V_DrawStringA(
				Font, 0, Buf,
				a_X + STS_SBX(Profile, 8, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
				a_Y + STS_SBY(Profile, 192, a_W, a_H) - 12 - (BigLetters ? 4 : 0)
			);
		
		//// ARMOR
		if (!IsMonster)
		{
			// Draw Armor Icon
			if (!DisplayP->armortype)
				vi = V_ImageFindA("sboempty", VCP_DOOM);
			else if (DisplayP->armortype == 1)
				vi = V_ImageFindA("sboarmwk", VCP_DOOM);
			else
				vi = V_ImageFindA("sboarmor", VCP_DOOM);
			if (vi)
				V_ImageDraw(
						0, vi,
						a_X + STS_SBX(Profile, 96, a_W, a_H),
						a_Y + STS_SBY(Profile, 192, a_W, a_H) - 16,
						NULL
					);
		
			// Draw Armor Text
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->armorpoints);
			V_DrawStringA(
					Font, 0, Buf,
					a_X + STS_SBX(Profile, 96, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
					a_Y + STS_SBY(Profile, 192, a_W, a_H) - 12 - (BigLetters ? 4 : 0)
				);
		}
		
		//// WEAPON/AMMO
		if (!IsMonster)
		{
			// Draw Icon
			vi = V_ImageFindA((DisplayP->weaponinfo && DisplayP->weaponinfo[ReadyWeapon]->SBOGraphic ? DisplayP->weaponinfo[ReadyWeapon]->SBOGraphic : "sboempty"), VCP_DOOM);
			if (vi)
				V_ImageDraw(
						0, vi,
						a_X + STS_SBX(Profile, 240, a_W, a_H),
						a_Y + STS_SBY(Profile, 192, a_W, a_H) - 16,
						NULL
					);
		
			// Draw Ammo Text
			if (DisplayP->ammo)
			{
				if (AmmoType < 0 || AmmoType >= NUMAMMO || P_XGSVal(PGS_PLINFINITEAMMO))
					snprintf(Buf, BUFSIZE - 1, "-");
				else
					snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->ammo[AmmoType]);
				V_DrawStringA(
						Font, 0, Buf,
						a_X + STS_SBX(Profile, 240, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
						a_Y + STS_SBY(Profile, 192, a_W, a_H) - 12 - (BigLetters ? 4 : 0)
					);
			}
		}
		
		//// FRAGS
		if (P_GMIsDM())
		{
			// Draw Icon
			vi = V_ImageFindA("sbofrags", VCP_DOOM);
			if (vi)
				V_ImageDraw(
						0, vi,
						a_X + STS_SBX(Profile, 240, a_W, a_H),
						a_Y + STS_SBY(Profile, 8, a_W, a_H),
						NULL
					);
		
			// Draw Frags Text
			snprintf(Buf, BUFSIZE - 1, "%i", DisplayP->TotalFrags);
			V_DrawStringA(
					Font, 0, Buf,
					a_X + STS_SBX(Profile, 240, a_W, a_H) + 20 - (BigLetters ? 0 : 2),
					a_Y + STS_SBY(Profile, 8, a_W, a_H) - (BigLetters ? 4 : 0)
				);
		}
		
		//// KEYS
		Right = 8;
		for (k = 0; k < 2; k++)
			for (i = 0; i < 2; i++)
				for (j = 0; j < 32; j++)
				{
					OK = false;
				
					// We actually own the key
					if (DisplayP->KeyCards[i] & (UINT32_C(1) << j))
					{
						if (!k)
							OK = true;
					}
				
					// Do not own key, see if flashing
					else if (k == 1)
					{
						// Flash about every half second
						if (DisplayP->KeyFlash[i][j] & 0x10)
							OK = true;
					}
				
					// OK to draw?
					if (OK)
					{
						// Too many keys in view?
						if (Right > 300)
							break;
					
						// Try to find the key
						DrawKey = INFO_KeyByGroupBit(i, j);
					
						// No key exists here?
						if (!DrawKey)
							continue;
					
						// No image supplied?
						if (!DrawKey->ImageName)
							vi = V_ImageFindA("RMD_UKEY", VCP_DOOM);
					
						// Otherwise, use the key image possibly
						else
							vi = V_ImageFindA(DrawKey->ImageName, VCP_NONE);
					
						// Draw the key
						if (vi)
						{
							V_ImageDraw(0, vi, a_X + STS_SBX(Profile, Right, a_W, a_H), a_Y + STS_SBY(Profile, 165, a_W, a_H), NULL);
					
							// Shift
							Right += vi->Width + 1;
						}
					}
				}
	}
	
	/* Classic Doom */
	else
	{
	}
	
	/* Draw Object Overlays */
	
	/* We are looking at another player */
	if (ConsoleP != DisplayP)
	{
		// Put warning if the player is under attack
			// TODO
	}
	
	/* Scoreboard */
	if ((XPlay && XPlay->Scores) || (a_PID == 0 && g_NetBoardDown))
	{
		WI_DrawScoreBoard(false, DS_GetString(DSTR_STSTUFFC_SCOREBOARD), NULL);
	}
#undef BUFSIZE
}

/*** FUNCTIONS ***/

/* ST_DrawPlayerBarsEx() -- Draw player status bars */
void ST_DrawPlayerBarsEx(void)
{
	player_t* ConsoleP, *DisplayP;
	int p, x, y, w, h;
	bool_t BigLetters;
	static uint32_t LastPal;	// Lowers palette change (faster drawing)
	
	/* Screen division? */
	// Initial
	x = y = 0;
	w = 320;
	h = 200;
	
	// 2+ split
	if (g_SplitScreen >= 1)
		h /= 2;
	
	// 3+ split
	if (g_SplitScreen >= 2)
		w /= 2;
		
	/* Use standard palette */
	if (g_SplitScreen > 0)
	{
		if (LastPal != 0)
			V_SetPalette(0);
		LastPal = 0;
	}
	
	/* Draw each player */
	for (p = 0; p < (g_SplitScreen < 0 ? 1 : g_SplitScreen + 1); p++)
	{
		// Split player active
		if (D_ScrSplitVisible(p) || (demoplayback && g_Splits[p].Active))
		{
			// Get players to draw for
			if (!g_Splits[p].XPlayer)
			{
				if (g_Splits[p].Console >= 0 && playeringame[g_Splits[p].Console])
					ConsoleP = &players[g_Splits[p].Console];
				else
					ConsoleP = NULL;
			}
			else
				ConsoleP = g_Splits[p].XPlayer->Player;//&players[g_Splits[p].Console];
			DisplayP = P_SpecGetPOV(p);//&players[g_Splits[p].Display];
			
			// Missing player?
			if (!ConsoleP)
				ConsoleP = P_SpecGet(p);
			
			// Modify palette?
			if (g_SplitScreen <= 0)	// Only 1 player inside
			{
				if (LastPal != DisplayP->PalChoice)
				{
					V_SetPalette(DisplayP->PalChoice);
					LastPal = DisplayP->PalChoice;
				}
			}
	
			// Draw Bar
			STS_DrawPlayerBarEx(p, x, y, w, h, ConsoleP, DisplayP);
		}
	
		// Add to coords (finished drawing everything, or not drawn at all)
		if (g_SplitScreen == 1)
			y += h;
		else if (g_SplitScreen > 1)
		{
			x += w;
		
			if (x == (w * 2))
			{
				x = 0;
				y += h;
			}
		}
	}
}

/* ST_InitEx() -- Initializes the extended status bar */
void ST_InitEx(void)
{
}

/* ST_TickerEx() -- Extended Ticker */
void ST_TickerEx(void)
{
	player_t* Player;
	int ChosePal = 0;
	int BaseDam = 0, BzFade;
	size_t p;
	bool_t ActiveMenu;
	
	/* Update for all players */
	for (p = 0; p < MAXPLAYERS; p++)
	{
		// Get player
		Player = &players[p];
		
		// No player here?
		if (!playeringame[p])
			continue;
		
		// Player has a menu open?
		ActiveMenu = M_ExPlayerUIActive(p);
		
		// Player Palette
			// Reset variables -- Otherwise palettes "stick"
		ChosePal = BaseDam = BzFade = 0;
		
			// Berserker fade
		BaseDam = Player->damagecount;
	
		if (Player->powers[pw_strength])
		{
			BzFade = 12 - (Player->powers[pw_strength] >> 6);
		
			if (BzFade > BaseDam)
				BaseDam = BzFade;
		}
	
			// Player is hurt?
		if (BaseDam)
		{
			// Division is number of palettes
			ChosePal = FixedMul(NUMREDPALS << FRACBITS, FixedDiv((BaseDam) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
			ChosePal++;				// +7
			//ChosePal = (plyr->damagecount * (10000 / NUMREDPALS) ) / 100;
			
			// Menu Cutdown
			if (ActiveMenu)
				ChosePal >>= 1;
			
			// Don't exceed
			if (ChosePal >= NUMREDPALS)
				ChosePal = NUMREDPALS - 1;
			
			// Offset
			ChosePal += STARTREDPALS;
		}
	
			// Player got an item
		else if (Player->bonuscount)
		{
			// Division is number of palettes
			ChosePal = FixedMul(NUMBONUSPALS << FRACBITS, FixedDiv((Player->bonuscount) << FRACBITS, 100 << FRACBITS)) >> FRACBITS;
			ChosePal++;				// +7
			
			// Menu Cutdown
			if (ActiveMenu)
				ChosePal >>= 1;
		
			// Don't exceed
			if (ChosePal >= NUMBONUSPALS)
				ChosePal = NUMBONUSPALS - 1;
			
			// Offset
			ChosePal += STARTBONUSPALS;
		}
	
			// Player has radiation suit
		else if (Player->powers[pw_ironfeet] > 4 * 32 || Player->powers[pw_ironfeet] & 8)
		{
			ChosePal = RADIATIONPAL;
		}
		
		// Set palette to what was chosen
		Player->PalChoice = ChosePal;
	}
}

/* ST_ExSoloViewTransSBar() -- Transparent status bar for single view */
bool_t ST_ExSoloViewTransSBar(void)
{
	return false;
}

/* ST_ExSoloViewScaledSBar() -- Scaled status bar for single view */
bool_t ST_ExSoloViewScaledSBar(void)
{
	return false;
}

/* ST_ExViewBarHeight() -- Status Bar Height */
int32_t ST_ExViewBarHeight(void)
{
	return 0;
}

/* ST_CheckDrawGameView() -- Checks if game view can be drawn */
bool_t ST_CheckDrawGameView(const int32_t a_Screen)
{
	/* Check */
	if (a_Screen < 0 || a_Screen >= MAXSPLITSCREEN)
		return false;	
	
	/* Automap disables view */
	if (g_Splits[a_Screen].AutomapActive)
		// Overlay automap draws the view
		if (g_Splits[a_Screen].OverlayMap)
			return true;
		else
			return false;
	
	/* Otherwise, draw it */
	return true;
}

