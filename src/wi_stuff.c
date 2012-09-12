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
//      Intermission screens.

#include "doomdef.h"
#include "wi_stuff.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "m_random.h"
#include "r_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "i_video.h"
#include "v_video.h"
#include "z_zone.h"
#include "console.h"
#include "p_info.h"
#include "dstrings.h"
#include "p_demcmp.h"

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//

//
// Different vetween registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMEPISODES     4
#define NUMMAPS         9

// GLOBAL LOCATIONS
#define WI_TITLEY               2
#define WI_SPACINGY             16	//TODO: was 33

// SINGPLE-PLAYER STUFF
#define SP_STATSX               50
#define SP_STATSY               50

#define SP_TIMEX                16
#define SP_TIMEY                (BASEVIDHEIGHT-32)

// NET GAME STUFF
#define NG_STATSY               50
#define NG_STATSX               (32 + LittleSwapInt16(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX             64

// DEATHMATCH STUFF
#define DM_MATRIXX              16
#define DM_MATRIXY              24

#define DM_SPACINGX             32

#define DM_TOTALSX              269

#define DM_KILLERSX             0
#define DM_KILLERSY             100
#define DM_VICTIMSX             5
#define DM_VICTIMSY             50
// in sec
#define DM_WAIT                 20

typedef enum
{
	ANIM_ALWAYS,
	ANIM_RANDOM,
	ANIM_LEVEL
} animenum_t;

typedef struct
{
	int x;
	int y;
	
} point_t;

//
// Animation.
// There is another wianim_t used in p_spec.
//
typedef struct
{
	animenum_t type;
	
	// period in tics between animations
	int period;
	
	// number of animation frames
	int nanims;
	
	// location of animation
	point_t loc;
	
	// ALWAYS: n/a,
	// RANDOM: period deviation (<256),
	// LEVEL: level
	int data1;
	
	// ALWAYS: n/a,
	// RANDOM: random base period,
	// LEVEL: n/a
	int data2;
	
	// actual graphics for frames of animations
	patch_t* p[3];
	
	// following must be initialized to zero before use!
	
	// next value of bcnt (used in conjunction with period)
	int nexttic;
	
	// last drawn animation frame
	int lastdrawn;
	
	// next frame number to animate
	int ctr;
	
	// used by RANDOM and LEVEL when animating
	int state;
	
} wianim_t;

static point_t doomlnodes[NUMEPISODES][NUMMAPS] =
{
	// Episode 0 World Map
	{
		{185, 164},				// location of level 0 (CJ)
		{148, 143},				// location of level 1 (CJ)
		{69, 122},					// location of level 2 (CJ)
		{209, 102},				// location of level 3 (CJ)
		{116, 89},					// location of level 4 (CJ)
		{166, 55},					// location of level 5 (CJ)
		{71, 56},					// location of level 6 (CJ)
		{135, 29},					// location of level 7 (CJ)
		{71, 24}					// location of level 8 (CJ)
	},
	
	// Episode 1 World Map should go here
	{
		{254, 25},					// location of level 0 (CJ)
		{97, 50},					// location of level 1 (CJ)
		{188, 64},					// location of level 2 (CJ)
		{128, 78},					// location of level 3 (CJ)
		{214, 92},					// location of level 4 (CJ)
		{133, 130},				// location of level 5 (CJ)
		{208, 136},				// location of level 6 (CJ)
		{148, 140},				// location of level 7 (CJ)
		{235, 158}					// location of level 8 (CJ)
	},
	
	// Episode 2 World Map should go here
	{
		{156, 168},				// location of level 0 (CJ)
		{48, 154},					// location of level 1 (CJ)
		{174, 95},					// location of level 2 (CJ)
		{265, 75},					// location of level 3 (CJ)
		{130, 48},					// location of level 4 (CJ)
		{279, 23},					// location of level 5 (CJ)
		{198, 48},					// location of level 6 (CJ)
		{140, 25},					// location of level 7 (CJ)
		{281, 136}					// location of level 8 (CJ)
	}
};

//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//
static wianim_t epsd0animinfo[] =
{
	{ANIM_ALWAYS, TICRATE / 3, 3, {224, 104}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {184, 160}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {112, 136}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {72, 112}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {88, 96}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {64, 48}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {192, 40}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {136, 16}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {80, 16}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {64, 24}}
};

static wianim_t epsd1animinfo[] =
{
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 1},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 2},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 3},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 4},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 5},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 6},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 7},
	{ANIM_LEVEL, TICRATE / 3, 3, {192, 144}, 8},
	{ANIM_LEVEL, TICRATE / 3, 1, {128, 136}, 8}
};

static wianim_t epsd2animinfo[] =
{
	{ANIM_ALWAYS, TICRATE / 3, 3, {104, 168}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {40, 136}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {160, 96}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {104, 80}},
	{ANIM_ALWAYS, TICRATE / 3, 3, {120, 32}},
	{ANIM_ALWAYS, TICRATE / 4, 3, {40, 0}}
};

static int NUMANIMS[NUMEPISODES] =
{
	sizeof(epsd0animinfo) / sizeof(wianim_t),
	sizeof(epsd1animinfo) / sizeof(wianim_t),
	sizeof(epsd2animinfo) / sizeof(wianim_t)
};

static wianim_t* wianims[NUMEPISODES] =
{
	epsd0animinfo,
	epsd1animinfo,
	epsd2animinfo
};

//
// GENERAL DATA
//

//
// Locally used stuff.
//
#define FB 0

// States for the intermission

typedef enum
{
	NoState = -1,
	StatCount,
	ShowNextLoc
} stateenum_t;

// States for single-player
#define SP_KILLS                0
#define SP_ITEMS                2
#define SP_SECRET               4
#define SP_FRAGS                6
#define SP_TIME                 8
#define SP_PAR                  ST_TIME

#define SP_PAUSE                1

// in seconds
#define SHOWNEXTLOCDELAY        4
//#define SHOWLASTLOCDELAY      SHOWNEXTLOCDELAY

// used to accelerate or skip a stage
static int acceleratestage;

// wbs->pnum
static int me;

// specifies current state
static stateenum_t state;

// contains information passed into intermission
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;	// wbs->plyr[]

// used for general timing
static int cnt;

// used for timing of background animation
static int bcnt;

// signals to refresh everything for one frame
static int firstrefresh;

static int cnt_kills[MAXPLAYERS];
static int cnt_items[MAXPLAYERS];
static int cnt_secret[MAXPLAYERS];
static int cnt_time;
static int cnt_par;
static int cnt_pause;

// # of commercial levels
static int NUMCMAPS;

//
//      GRAPHICS
//

// background (map of levels).
//static patch_t*       bg;
static char bgname[9];

// You Are Here graphic
static patch_t* yah[2];

// splat
static patch_t* splat;

// %, : graphics
static patch_t* percent;
static patch_t* colon;

// 0-9 graphic
static patch_t* num[10];

// minus sign
static patch_t* wiminus;

// "Finished!" graphics
static patch_t* finished;

// "Entering" graphic
static patch_t* entering;

// "secret"
static patch_t* sp_secret;

// "Kills", "Scrt", "Items", "Frags"
static patch_t* kills;
static patch_t* secret;
static patch_t* items;
static patch_t* frags;

// Time sucks.
static patch_t* timePatch;
static patch_t* par;
static patch_t* sucks;

// "killers", "victims"
static patch_t* killers;
static patch_t* victims;

// "Total", your face, your dead face
static patch_t* total;
static patch_t* star;
static patch_t* bstar;

//added:08-02-98: use STPB0 for all players, but translate the colors
static patch_t* stpb;

// Name graphics of each level (centered)
static patch_t** lnames;

//
// CODE
//

/****************
*** CONSTANTS ***
****************/

#define WIDPLIMIT 12

/*****************
*** STRUCTURES ***
*****************/

/* WI_PlayerInfo_t -- Player Info */
typedef struct WI_PlayerInfo_s
{
	player_t* Player;
	int32_t Rank;
	bool_t LocalPlayer;
	int8_t ScreenNum;
	char PlayerName[MAXPLAYERNAME + 1];
	
	/* Stats */
	int32_t Kills;
	int32_t Items;
	int32_t Secrets;
	
	int32_t KillPcnt;
	int32_t ItemPcnt;
	int32_t SecretPcnt;
	
	/* Percent Pointers */
	int* cntKillsPtr;
	int* cntItemsPtr;
	int* cntSecretsPtr;
} WI_PlayerInfo_t;

/*************
*** LOCALS ***
*************/

static V_Image_t* l_PicINTER = NULL;
static WI_PlayerInfo_t l_DrawPlayers[MAXPLAYERS + 1];
static size_t l_NumDrawPlayers;
static int32_t l_TotalKills, l_TotalItems, l_TotalSecrets;

/****************
*** FUNCTIONS ***
****************/

/* WI_slamBackground() -- Slams intermission background on the screen */
static void WI_slamBackground(void)
{
	/* Copy old screen */
	memcpy(screens[0], screens[1], vid.width * vid.height);
	
	/* Draw Interpic */
	V_ImageDraw(0, l_PicINTER, 0, 0, NULL);
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
bool_t WI_Responder(event_t* ev)
{
	return false;
}

// Draws "<Levelname> Finished!"
static void WI_drawLF(void)
{
}

// Draws "Entering <LevelName>"
static void WI_drawEL(void)
{
}

/* WI_drawOnLnode() -- Draws level currently on */
static void WI_drawOnLnode(int n, patch_t* c[])
{
}

static void WI_initAnimatedBack(void)
{
	int i;
	wianim_t* a;
	
	if (gamemode == commercial)
		return;
		
	if (wbs->epsd > 2)
		return;
		
	for (i = 0; i < NUMANIMS[wbs->epsd]; i++)
	{
		a = &wianims[wbs->epsd][i];
		
		// init variables
		a->ctr = -1;
		
		// specify the next time to draw it
		if (a->type == ANIM_ALWAYS)
			a->nexttic = bcnt + 1 + (M_Random() % a->period);
		else if (a->type == ANIM_RANDOM)
			a->nexttic = bcnt + 1 + a->data2 + (M_Random() % a->data1);
		else if (a->type == ANIM_LEVEL)
			a->nexttic = bcnt + 1;
	}
	
}

static void WI_updateAnimatedBack(void)
{
	int i;
	wianim_t* a;
	
	if (gamemode == commercial)
		return;
		
	if (wbs->epsd > 2)
		return;
		
	for (i = 0; i < NUMANIMS[wbs->epsd]; i++)
	{
		a = &wianims[wbs->epsd][i];
		
		if (bcnt >= a->nexttic)
		{
			switch (a->type)
			{
				case ANIM_ALWAYS:
					if (++a->ctr >= a->nanims)
						a->ctr = 0;
					a->nexttic = bcnt + a->period;
					break;
					
				case ANIM_RANDOM:
					a->ctr++;
					if (a->ctr == a->nanims)
					{
						a->ctr = -1;
						a->nexttic = bcnt + a->data2 + (M_Random() % a->data1);
					}
					else
						a->nexttic = bcnt + a->period;
					break;
					
				case ANIM_LEVEL:
					// gawd-awful hack for level wianims
					if (!(state == StatCount && i == 7) && wbs->next == a->data1)
					{
						a->ctr++;
						if (a->ctr == a->nanims)
							a->ctr--;
						a->nexttic = bcnt + a->period;
					}
					break;
			}
		}
		
	}
	
}

/* WI_drawAnimatedBack() -- Draws animated background */
static void WI_drawAnimatedBack(void)
{
}

/* WI_drawNum() -- Draws number */
static int WI_drawNum(int x, int y, int n, int digits)
{
	return 0;
}

/* WI_drawPercent() -- Draws percentage */
static void WI_drawPercent(int x, int y, int p)
{
}

/* WI_drawTime() -- Draws the time */
static void WI_drawTime(int x, int y, int t)
{
}

static void WI_unloadData(void);

static void WI_End(void)
{
	WI_unloadData();
}

// used for write introduce next level
static void WI_initNoState(void)
{
	state = NoState;
	acceleratestage = 0;
	cnt = 10;
}

static void WI_updateNoState(void)
{

	WI_updateAnimatedBack();
	
	if (--cnt == 0)
	{
		WI_End();
		G_NextLevel();
	}
	
}

static bool_t snl_pointeron = false;

static void WI_initShowNextLoc(void)
{
	state = ShowNextLoc;
	acceleratestage = 0;
	cnt = SHOWNEXTLOCDELAY * TICRATE;
	
	WI_initAnimatedBack();
}

static void WI_updateShowNextLoc(void)
{
	WI_updateAnimatedBack();
	
	if (!--cnt || acceleratestage)
		WI_initNoState();
	else
		snl_pointeron = (cnt & 31) < 20;
}

/* WI_drawShowNextLoc() -- Draws area going to */
static void WI_drawShowNextLoc(void)
{
}

static void WI_drawNoState(void)
{
}

static int dm_frags[MAXPLAYERS][MAXPLAYERS];
static int dm_totals[MAXPLAYERS];

static void WI_initDeathmatchStats(void)
{

	int i;
	int j;
	
	state = StatCount;
	acceleratestage = 0;
	
	cnt_pause = TICRATE * DM_WAIT;
	
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			for (j = 0; j < MAXPLAYERS; j++)
				if (playeringame[j])
					dm_frags[i][j] = plrs[i].frags[j];
					
			dm_totals[i] = ST_PlayerFrags(i);
		}
		
	WI_initAnimatedBack();
}

static void WI_updateDeathmatchStats(void)
{
	WI_updateAnimatedBack();
	
	if (paused)
		return;
	if (cnt_pause > 0)
		cnt_pause--;
	if (cnt_pause == 0)
	{
		S_StartSound(0, sfx_slop);
		
		WI_initNoState();
	}
}

/* WI_drawRancking() -- Draws rankings */
void WI_drawRancking(char* title, int x, int y, fragsort_t* fragtable, int scorelines, bool_t large, int white)
{
}

#define RANKINGY 60

/* WI_drawDeathmatchStats() -- Draws Deathmatch stats */
static void WI_drawDeathmatchStats(void)
{
}

bool_t teamingame(int teamnum)
{
	int i;
	
	if (P_XGSVal(PGS_GAMETEAMPLAY) == 1)
	{
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].skincolor == teamnum)
				return true;
	}
	else if (P_XGSVal(PGS_GAMETEAMPLAY) == 2)
	{
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && players[i].skin == teamnum)
				return true;
	}
	return false;
}

/* WI_drawTeamsStats() -- Draws team stats */
static void WI_drawTeamsStats(void)
{
}

/* old code
static void WI_ddrawDeathmatchStats(void)
{

    int         i;
    int         j;
    int         x;
    int         y;
    int         w;

    int         lh;     // line height

    uint8_t*       colormap;       //added:08-02-98:see below

    lh = WI_SPACINGY;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack();
    WI_drawLF();

    // draw stat titles (top line)
    V_DrawScaledPatch(DM_TOTALSX-LittleSwapInt16(total->width)/2,
                DM_MATRIXY-WI_SPACINGY+10,
                FB,
                total);

    V_DrawScaledPatch(DM_KILLERSX, DM_KILLERSY, FB, killers);
    V_DrawScaledPatch(DM_VICTIMSX, DM_VICTIMSY, FB, victims);

    // draw P?
    x = DM_MATRIXX + DM_SPACINGX;
    y = DM_MATRIXY;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            //added:08-02-98: use V_DrawMappedPatch instead of
            //                    V_DrawScaledPatch, so that the
            // graphics are 'colormapped' to the player's colors!
            if (players[i].skincolor==0)
                colormap = colormaps;
            else
                colormap = (uint8_t *) translationtables - 256 + (players[i].skincolor<<8);

            V_DrawMappedPatch(x-LittleSwapInt16(stpb->width)/2,
                        DM_MATRIXY - WI_SPACINGY,
                        FB,
                        stpb,      //p[i], now uses a common STPB0 translated
                        colormap); //      to the right colors

            V_DrawMappedPatch(DM_MATRIXX-LittleSwapInt16(stpb->width)/2,
                        y,
                        FB,
                        stpb,      //p[i]
                        colormap);

            if (i == me)
            {
                V_DrawScaledPatch(x-LittleSwapInt16(stpb->width)/2,
                            DM_MATRIXY - WI_SPACINGY,
                            FB,
                            bstar);

                V_DrawScaledPatch(DM_MATRIXX-LittleSwapInt16(stpb->width)/2,
                            y,
                            FB,
                            star);
            }
        }
        else
        {
            // V_DrawPatch(x-LittleSwapInt16(bp[i]->width)/2,
            //   DM_MATRIXY - WI_SPACINGY, FB, bp[i]);
            // V_DrawPatch(DM_MATRIXX-LittleSwapInt16(bp[i]->width)/2,
            //   y, FB, bp[i]);
        }
        x += DM_SPACINGX;
        y += WI_SPACINGY;
    }

    // draw stats
    y = DM_MATRIXY+10;
    w = LittleSwapInt16(num[0]->width);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        x = DM_MATRIXX + DM_SPACINGX;

        if (playeringame[i])
        {
            for (j=0 ; j<MAXPLAYERS ; j++)
            {
                if (playeringame[j])
                    WI_drawNum(x+w, y, dm_frags[i][j], 2);

                x += DM_SPACINGX;
            }
            WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
        }
        y += WI_SPACINGY;
    }
}

*/

static int cnt_frags[MAXPLAYERS];
static int dofrags;
static int ng_state;

static void WI_initNetgameStats(void)
{

	int i;
	
	state = StatCount;
	acceleratestage = 0;
	ng_state = 1;
	
	cnt_pause = TICRATE;
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;
			
		cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;
		
		dofrags += ST_PlayerFrags(i);
	}
	
	dofrags = ! !dofrags;
	
	WI_initAnimatedBack();
}

static void WI_updateNetgameStats(void)
{

	int i;
	int fsum;
	
	bool_t stillticking;
	
	WI_updateAnimatedBack();
	
	if (acceleratestage && ng_state != 10)
	{
		acceleratestage = 0;
		
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;
				
			cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
			cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
			cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
			
			if (dofrags)
				cnt_frags[i] = ST_PlayerFrags(i);
		}
		S_StartSound(0, sfx_barexp);
		ng_state = 10;
	}
	
	if (ng_state == 2)
	{
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		stillticking = false;
		
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;
				
			cnt_kills[i] += 2;
			
			if (cnt_kills[i] >= (plrs[i].skills * 100) / wbs->maxkills)
				cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
			else
				stillticking = true;
		}
		
		if (!stillticking)
		{
			S_StartSound(0, sfx_barexp);
			ng_state++;
		}
	}
	else if (ng_state == 4)
	{
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		stillticking = false;
		
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;
				
			cnt_items[i] += 2;
			if (cnt_items[i] >= (plrs[i].sitems * 100) / wbs->maxitems)
				cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
			else
				stillticking = true;
		}
		if (!stillticking)
		{
			S_StartSound(0, sfx_barexp);
			ng_state++;
		}
	}
	else if (ng_state == 6)
	{
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		stillticking = false;
		
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;
				
			cnt_secret[i] += 2;
			
			if (cnt_secret[i] >= (plrs[i].ssecret * 100) / wbs->maxsecret)
				cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
			else
				stillticking = true;
		}
		
		if (!stillticking)
		{
			S_StartSound(0, sfx_barexp);
			ng_state += 1 + 2 * !dofrags;
		}
	}
	else if (ng_state == 8)
	{
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		stillticking = false;
		
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;
				
			cnt_frags[i] += 1;
			
			if (cnt_frags[i] >= (fsum = ST_PlayerFrags(i)))
				cnt_frags[i] = fsum;
			else
				stillticking = true;
		}
		
		if (!stillticking)
		{
			S_StartSound(0, sfx_pldeth);
			ng_state++;
		}
	}
	else if (ng_state == 10)
	{
		if (acceleratestage)
		{
			S_StartSound(0, sfx_sgcock);
			if (gamemode == commercial)
				WI_initNoState();
			else
				WI_initShowNextLoc();
		}
	}
	else if (ng_state & 1)
	{
		if (!--cnt_pause)
		{
			ng_state++;
			cnt_pause = TICRATE;
		}
	}
}

/* WI_drawStats() -- Draws Co-Op Stats */
static void WI_drawNetgameStats(void)
{
}

static int sp_state;

static void WI_initStats(void)
{
	state = StatCount;
	acceleratestage = 0;
	sp_state = 1;
	cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
	cnt_time = cnt_par = -1;
	cnt_pause = TICRATE;
	
	WI_initAnimatedBack();
}

static void WI_updateStats(void)
{

	WI_updateAnimatedBack();
	
	if (acceleratestage && sp_state != 10)
	{
		acceleratestage = 0;
		cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
		cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
		cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
		cnt_time = plrs[me].stime / TICRATE;
		cnt_par = wbs->partime / TICRATE;
		S_StartSound(0, sfx_barexp);
		sp_state = 10;
	}
	
	if (sp_state == 2)
	{
		cnt_kills[0] += 2;
		
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
		{
			cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
			S_StartSound(0, sfx_barexp);
			sp_state++;
		}
	}
	else if (sp_state == 4)
	{
		cnt_items[0] += 2;
		
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
		{
			cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
			S_StartSound(0, sfx_barexp);
			sp_state++;
		}
	}
	else if (sp_state == 6)
	{
		cnt_secret[0] += 2;
		
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
		{
			cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
			S_StartSound(0, sfx_barexp);
			sp_state++;
		}
	}
	
	else if (sp_state == 8)
	{
		if (!(bcnt & 3))
			S_StartSound(0, sfx_pistol);
			
		cnt_time += 3;
		
		if (cnt_time >= plrs[me].stime / TICRATE)
			cnt_time = plrs[me].stime / TICRATE;
			
		cnt_par += 3;
		
		if (cnt_par >= wbs->partime / TICRATE)
		{
			cnt_par = wbs->partime / TICRATE;
			
			if (cnt_time >= plrs[me].stime / TICRATE)
			{
				S_StartSound(0, sfx_barexp);
				sp_state++;
			}
		}
	}
	else if (sp_state == 10)
	{
		if (acceleratestage)
		{
			S_StartSound(0, sfx_sgcock);
			
			if (gamemode == commercial)
				WI_initNoState();
			else
				WI_initShowNextLoc();
		}
	}
	else if (sp_state & 1)
	{
		if (!--cnt_pause)
		{
			sp_state++;
			cnt_pause = TICRATE;
		}
	}
	
}

/* WI_drawStats() -- Draws Single Player Stats */
static void WI_drawStats(void)
{
}

static void WI_checkForAccelerate(void)
{
	int i;
	player_t* player;
	
	// check for button presses to skip delays
	for (i = 0, player = players; i < MAXPLAYERS; i++, player++)
	{
		if (playeringame[i])
		{
			if (player->cmd.Std.buttons & BT_ATTACK)
			{
				if (!player->attackdown)
					acceleratestage = 1;
				player->attackdown = true;
			}
			else
				player->attackdown = false;
			if (player->cmd.Std.buttons & BT_USE)
			{
				if (!player->usedown)
					acceleratestage = 1;
				player->usedown = true;
			}
			else
				player->usedown = false;
		}
	}
}

// Updates stuff each tick
void WI_Ticker(void)
{
	// GhostlyDeath <May 6, 2012> -- Player tic update
	D_NCSNetUpdateSingleTic();	
	
	// Update music
	I_UpdateMusic();
	
	// counter for general background animation
	bcnt++;
	
	WI_checkForAccelerate();
	
	switch (state)
	{
		case StatCount:
			if (P_XGSVal(PGS_GAMEDEATHMATCH))
				WI_updateDeathmatchStats();
			else if (multiplayer)
				WI_updateNetgameStats();
			else
				WI_updateStats();
			break;
			
		case ShowNextLoc:
			WI_updateShowNextLoc();
			break;
			
		case NoState:
			WI_updateNoState();
			break;
	}
	
}

/* WI_loadData() -- Loads Intermission Data */
static void WI_loadData(void)
{
	/* Load level-based info */
	if (g_CurrentLevelInfo)
	{
		// INTERPIC
		if (g_CurrentLevelInfo->InterPic)
			l_PicINTER = V_ImageFindA(g_CurrentLevelInfo->InterPic, VCP_NONE);
	}
	
	
}

/* WI_unloadData() -- Unloads intermission Data */
static void WI_unloadData(void)
{
	/* Delete everything */
	if (l_PicINTER)
		V_ImageDestroy(l_PicINTER);
}

/* WI_Drawer() -- Draws the intermission */
void WI_Drawer(void)
{
#define BUFSIZE 64
	char Buf[BUFSIZE];
	size_t i, j;
	int32_t xBase, yBase, yAdd, y, dp, k;
	bool_t IsOnScreen;
	uint32_t DrawFlags;
	int8_t ScreenNum;
	const char* Title;
	int32_t Val, pVal;
	bool_t Flash, All;
	fixed_t mVal;
	
	/* Generic Drawing */
	// Draw interpic
	V_ImageDraw(0, l_PicINTER, 0, 0, NULL);
	
	/* Draw Level Name */
	if (g_CurrentLevelInfo)
	{
		// Use title?
		if (g_CurrentLevelInfo->Title)
		{
			V_DrawStringA(
				VFONT_LARGE,
				VFO_COLOR(VEX_MAP_BRIGHTWHITE),
				g_CurrentLevelInfo->Title,
				5, 5
			);
			
			// Draw Author?
			if (g_CurrentLevelInfo->Author)
				V_DrawStringA(
					VFONT_SMALL,
					VFO_COLOR(VEX_MAP_GREEN),
					g_CurrentLevelInfo->Author,
					5, 5 + V_FontHeight(VFONT_LARGE) + 1
				);
		}
		
		// Otherwise use level picture
		else if (g_CurrentLevelInfo->LevelPic)
		{
		}
	}
	
	/* Generalized Drawing */
	// Determine lines
	xBase = 10;
	yBase = 30;
	yAdd = V_FontHeight(VFONT_SMALL) + 1;
	
	if (l_NumDrawPlayers == 1)
		Flash = false;
	else
		Flash = true;
	
	// Draw player sort
	for (y = 0, i = 0; i < l_NumDrawPlayers + 3; i++, y += yAdd + 1)
	{
		// Clear
		DrawFlags = 0;
		dp = i;
		dp--;
		All = false;
		
		// Drawing A Player
		if (dp >= 0 && dp < l_NumDrawPlayers)
		{
			// Color based on rank
#if 0
				// Gold -- First
			if (Flash && l_DrawPlayers[dp].Rank == 0)
				DrawFlags = VFO_COLOR(VEX_MAP_GOLD);
				// Silver -- Second
			else if (Flash && l_DrawPlayers[dp].Rank == 1)
				DrawFlags = VFO_COLOR(VEX_MAP_WHITE);
				// Bronze -- Third
			else if (Flash && l_DrawPlayers[dp].Rank == 2)
				DrawFlags = VFO_COLOR(VEX_MAP_BROWN);
				// All other places are insignifant
			else
#endif
			{
				// Local Players are distinguished
				if (l_DrawPlayers[dp].LocalPlayer)
					DrawFlags = VFO_COLOR(VEX_MAP_BRIGHTWHITE);
				
				// Others are not
				else
					DrawFlags = VFO_COLOR(VEX_MAP_RED);
			}
			
			// Draw player band (their skin color)
			if (l_DrawPlayers[dp].Player)
				V_DrawColorBoxEx(
						VEX_TRANS(VEX_TRANS50) | VEX_COLORMAP(VEX_MAP_GREEN) |
							VEX_PCOLOR(l_DrawPlayers[dp].Player->skincolor),
						120, 0, yBase + y, 320, yBase + y + yAdd);
			
			// Draw Rank
			snprintf(Buf, BUFSIZE - 1, "%i.", l_DrawPlayers[dp].Rank + 1);
			V_DrawStringA(
					VFONT_SMALL,
					DrawFlags,
					Buf,
					xBase, yBase + y
				);
		
			// Draw player name
			if (l_DrawPlayers[dp].LocalPlayer)
				snprintf(Buf, BUFSIZE - 1, "%s (P%i)", l_DrawPlayers[dp].PlayerName, l_DrawPlayers[dp].ScreenNum + 1);
			else
				snprintf(Buf, BUFSIZE - 1, "%s", l_DrawPlayers[dp].PlayerName);
			V_DrawStringA(
					VFONT_SMALL,
					DrawFlags,
					Buf,
					xBase + 20, yBase + y
				);
		}
		
		// Headers are of different color
		else if (dp < 0)
		{
			DrawFlags = VFO_COLOR(VEX_MAP_RED);
			
			// Draw Players Header
			snprintf(Buf, BUFSIZE - 1, "%s", "PLAYERS");
			V_DrawStringA(
					VFONT_SMALL,
					DrawFlags,
					Buf,
					xBase + 20, yBase + y
				);
		}
		
		// As are footers
		else if (dp == l_NumDrawPlayers)
			DrawFlags = VFO_COLOR(VEX_MAP_YELLOW);
			
		// Draw Map Totals
		else
		{
			DrawFlags = VFO_COLOR(VEX_MAP_GREEN);
			All = true;
			
			// Draw Totals Header
			snprintf(Buf, BUFSIZE - 1, "%s", "MAP TOTALS");
			V_DrawStringA(
					VFONT_SMALL,
					DrawFlags,
					Buf,
					xBase + 20, yBase + y
				);
		}
		
		// Single-player/Cooperative
		if (!P_XGSVal(PGS_GAMEDEATHMATCH))
		{
			for (k = 0; k < 3; k++)
			{
				Val = 0;
				
				// Secrets
				if (k == 0)
				{
					// Get Multiplier
					mVal = FIXEDT_C(1) << FRACBITS;
					if (dp < l_NumDrawPlayers)
						if (l_DrawPlayers[dp].cntSecretsPtr)
							mVal = FixedDiv(
									(fixed_t)*l_DrawPlayers[dp].cntSecretsPtr,
									(fixed_t)l_DrawPlayers[dp].SecretPcnt);
					
					// Get Value
					if (dp >= 0)
						Val = (dp < l_NumDrawPlayers ? l_DrawPlayers[dp].Secrets :
								(!All ? l_TotalSecrets : totalsecret));
								
					// Multiply by percent
					Val = FixedMul(Val << FRACBITS, mVal) >> FRACBITS;
					if (Val < 0)
						Val = 0;
					
					Title = "SCRT";
				}
				
				// Items
				else if (k == 1)
				{
					// Get Multiplier
					mVal = FIXEDT_C(1) << FRACBITS;
					if (dp < l_NumDrawPlayers)
						if (l_DrawPlayers[dp].cntItemsPtr)
							mVal = FixedDiv(
									(fixed_t)*l_DrawPlayers[dp].cntItemsPtr,
									(fixed_t)l_DrawPlayers[dp].ItemPcnt);
					
					// Get Value
					if (dp >= 0)
						Val = (dp < l_NumDrawPlayers ? l_DrawPlayers[dp].Items :
								(!All ? l_TotalItems : totalitems));
								
					// Multiply by percent
					Val = FixedMul(Val << FRACBITS, mVal) >> FRACBITS;
					if (Val < 0)
						Val = 0;
					
					Title = "ITEM";
				}
				
				// Kills
				else if (k == 2)
				{
					// Get Multiplier
					mVal = FIXEDT_C(1) << FRACBITS;
					if (dp < l_NumDrawPlayers)
						if (l_DrawPlayers[dp].cntKillsPtr)
							mVal = FixedDiv(
									(fixed_t)*l_DrawPlayers[dp].cntKillsPtr,
									(fixed_t)l_DrawPlayers[dp].KillPcnt);
					
					// Get Value
					if (dp >= 0)
						Val = (dp < l_NumDrawPlayers ? l_DrawPlayers[dp].Kills :
								(!All ? l_TotalKills : totalkills));
					
					// Multiply by percent
					Val = FixedMul(Val << FRACBITS, mVal) >> FRACBITS;
					if (Val < 0)
						Val = 0;
					
					Title = "KILL";
				}
				
				// Which to draw?
				if (dp >= 0)
					snprintf(Buf, BUFSIZE - 1, "%i", Val);
				else
					snprintf(Buf, BUFSIZE - 1, "%s", Title);
				
				// Draw
				V_DrawStringA(
						VFONT_SMALL,
						DrawFlags,
						Buf,
						(320 - (xBase << 1)) - (35 * (k + 1)), yBase + y
					);
			}
		}
		
		// Deathmatch
		else
		{
		}
	}
	
#if 0
	switch (state)
	{
		case StatCount:
			if (cv_deathmatch.value)
			{
				if (cv_teamplay.value)
					WI_drawTeamsStats();
				else
					WI_drawDeathmatchStats();
			}
			else if (multiplayer)
				WI_drawNetgameStats();
			else
				WI_drawStats();
			break;
			
		case ShowNextLoc:
			WI_drawShowNextLoc();
			break;
			
		case NoState:
			WI_drawNoState();
			break;
	}
#endif
#undef BUFSIZE
}

/* WI_initVariables() -- Initializes variables */
static void WI_initVariables(wbstartstruct_t* wbstartstruct)
{
	size_t i, j, k;
	WI_PlayerInfo_t TempDP[MAXPLAYERS + 1];
	size_t NumTempDP;
	player_t* Player;
	
	wbs = wbstartstruct;
	
#ifdef RANGECHECKING
	if (gamemode != commercial)
	{
		if (gamemode == retail)
			RNGCHECK(wbs->epsd, 0, 3);
		else
			RNGCHECK(wbs->epsd, 0, 2);
	}
	else
	{
		RNGCHECK(wbs->last, 0, 8);
		RNGCHECK(wbs->next, 0, 8);
	}
	RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif
	
	acceleratestage = 0;
	cnt = bcnt = 0;
	firstrefresh = 1;
	me = wbs->pnum;
	plrs = wbs->plyr;
	
	if (!wbs->maxkills)
		wbs->maxkills = 1;
		
	if (!wbs->maxitems)
		wbs->maxitems = 1;
		
	if (!wbs->maxsecret)
		wbs->maxsecret = 1;
		
	if (gamemode != retail)
		if (wbs->epsd > 2)
			wbs->epsd -= 3;
	
	/* Determine Players to Draw */
	// Clear
	memset(l_DrawPlayers, 0, sizeof(l_DrawPlayers));
	l_NumDrawPlayers = 0;
	l_TotalKills = l_TotalItems = l_TotalSecrets = 0;
	
	memset(TempDP, 0, sizeof(TempDP));
	NumTempDP = 0;
	
	// Use players in game
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
		{
			Player = TempDP[NumTempDP].Player = &players[i];
			
			// Add totals
			l_TotalKills += Player->killcount;
			l_TotalItems += Player->itemcount;
			l_TotalSecrets += Player->secretcount;
			
			// Determine if is local player (on screen)
			for (k = 0; k < MAXSPLITSCREEN; k++)
				if (g_Splits[k].Active && i == g_Splits[k].Console)
				{
					TempDP[NumTempDP].LocalPlayer = true;
					TempDP[NumTempDP].ScreenNum = k;
					break;
				}
			
			// True Percentage for kills, items, secrets, etc.
			TempDP[NumTempDP].KillPcnt = (plrs[i].skills * 100) / wbs->maxkills;
			TempDP[NumTempDP].ItemPcnt = (plrs[i].sitems * 100) / wbs->maxitems;
			TempDP[NumTempDP].SecretPcnt = (plrs[i].ssecret * 100) / wbs->maxsecret;
			
			// Setup fields
			strncpy(TempDP[NumTempDP].PlayerName, D_NCSGetPlayerName(i), MAXPLAYERNAME - 1);
			TempDP[NumTempDP].Kills = Player->killcount;
			TempDP[NumTempDP].Items = Player->itemcount;
			TempDP[NumTempDP].Secrets = Player->secretcount;
			TempDP[NumTempDP].cntKillsPtr = &cnt_kills[i];
			TempDP[NumTempDP].cntItemsPtr = &cnt_items[i];
			TempDP[NumTempDP].cntSecretsPtr = &cnt_secret[i];
			
			TempDP[NumTempDP++].Rank = i;
		}
	
	// Un-Claimed Kills/Items/Secrets?
	if (!P_XGSVal(PGS_GAMEDEATHMATCH))
		if (l_TotalKills < g_MapKIS[0] || l_TotalItems < g_MapKIS[1] || l_TotalSecrets < g_MapKIS[2])
		{
			strncpy(TempDP[NumTempDP].PlayerName, "Un-Claimed", MAXPLAYERNAME - 1);
			
			// Kills
			if (l_TotalKills < g_MapKIS[0])
				TempDP[NumTempDP].Kills = g_MapKIS[0] - l_TotalKills;
			
			// Items
			if (l_TotalItems < g_MapKIS[1])
				TempDP[NumTempDP].Items = g_MapKIS[1] - l_TotalItems;
			
			// Secrets
			if (l_TotalSecrets < g_MapKIS[2])
				TempDP[NumTempDP].Secrets = g_MapKIS[2] - l_TotalSecrets;
			
			// Rank Last Always
			TempDP[NumTempDP++].Rank = i;
		}
	
	l_TotalKills = g_MapKIS[0];
	l_TotalItems = g_MapKIS[1];
	l_TotalSecrets = g_MapKIS[2];
	
	// Sort players based on statistics
	
	// Move into draw players, limiting to 16
	for (i = 0; i < NumTempDP; i++)
	{
		// Enough Room?
		if (l_NumDrawPlayers < WIDPLIMIT)
			l_DrawPlayers[l_NumDrawPlayers++] = TempDP[i];
		
		// Bump a non-local player out (but never replace 1st place)
		else if (TempDP[i].Player->NetPlayer && TempDP[i].Player->NetPlayer->Type == DNPT_LOCAL)
		{
			// If the player is NOT a local player
			for (j = WIDPLIMIT - 1; j > 1; j--)
				if ((!l_DrawPlayers[j].Player->NetPlayer) || (l_DrawPlayers[j].Player->NetPlayer && l_DrawPlayers[j].Player->NetPlayer->Type != DNPT_LOCAL))
				{
					l_DrawPlayers[j] = TempDP[i];
					break;
				}
		}
	}
}

/* WI_Start() -- Starts the intermission */
void WI_Start(wbstartstruct_t* wbstartstruct)
{
	/* Play Intermission Music */
	if (g_CurrentLevelInfo)
		if (g_CurrentLevelInfo->InterMus)
			S_ChangeMusicName(g_CurrentLevelInfo->InterMus, 1);
	
	/* Load Data */
	WI_initVariables(wbstartstruct);
	WI_loadData();
	
	/* Initialize Stats */
	if (P_XGSVal(PGS_GAMEDEATHMATCH))
		WI_initDeathmatchStats();
	else if (multiplayer)
		WI_initNetgameStats();
	else
		WI_initStats();
}

