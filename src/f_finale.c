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
//      Game completion, final screen animation.

#include "doomdef.h"
#include "doomstat.h"
#include "am_map.h"
#include "dstrings.h"
#include "d_main.h"
#include "f_finale.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "r_local.h"
#include "s_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "p_info.h"

// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast
int finalestage;

int finalecount;

#define TEXTSPEED       3
#define TEXTWAIT        250

char* finaletext;
char* finaleflat;
static bool_t keypressed = false;

void F_StartCast(void);
void F_CastTicker(void);
bool_t F_CastResponder(event_t* ev);
void F_CastDrawer(void);

//
// F_StartFinale
//
void F_StartFinale(void)
{
#if 0
	gamestate = GS_FINALE;
	
	if (info_intertext)
	{
		//SoM: Use FS level info intermission.
		finaletext = info_intertext;
		if (info_backdrop)
			finaleflat = info_backdrop;
		else
			finaleflat = text[FLOOR4_8_NUM];
			
		finalestage = 0;
		finalecount = 0;
		return;
	}
	// Okay - IWAD dependend stuff.
	// This has been changed severly, and
	//  some stuff might have changed in the process.
	switch (gamemode)
	{
	
			// DOOM 1 - E1, E3 or E4, but each nine missions
		case shareware:
		case registered:
		case retail:
			{
				S_ChangeMusic(mus_victor, true);
				
				switch (gameepisode)
				{
					case 1:
						finaleflat = text[FLOOR4_8_NUM];
						finaletext = E1TEXT;
						break;
					case 2:
						finaleflat = text[SFLR6_1_NUM];
						finaletext = E2TEXT;
						break;
					case 3:
						finaleflat = text[MFLR8_4_NUM];
						finaletext = E3TEXT;
						break;
					case 4:
						finaleflat = text[MFLR8_3_NUM];
						finaletext = E4TEXT;
						break;
					default:
						// Ouch.
						break;
				}
				break;
			}
			
			// DOOM II and missions packs with E1, M34
		case commercial:
			{
				S_ChangeMusic(mus_read_m, true);
				
				switch (gamemap)
				{
					case 6:
						finaleflat = text[SLIME16_NUM];
						finaletext = C1TEXT;
						break;
					case 11:
						finaleflat = text[RROCK14_NUM];
						finaletext = C2TEXT;
						break;
					case 20:
						finaleflat = text[RROCK07_NUM];
						finaletext = C3TEXT;
						break;
					case 30:
						finaleflat = text[RROCK17_NUM];
						finaletext = C4TEXT;
						break;
					case 15:
						finaleflat = text[RROCK13_NUM];
						finaletext = C5TEXT;
						break;
					case 31:
						finaleflat = text[RROCK19_NUM];
						finaletext = C6TEXT;
						break;
					default:
						// Ouch.
						break;
				}
				break;
			}
			
			// Indeterminate.
		default:
			S_ChangeMusic(mus_read_m, true);
			finaleflat = "F_SKY1";	// Not used anywhere else.
			finaletext = C1TEXT;	// FIXME - other text, music?
			break;
	}
	
	finalestage = 0;
	finalecount = 0;
#endif
}

bool_t F_Responder(event_t* event)
{
	if (finalestage == 2)
		return F_CastResponder(event);
	if (finalestage == 0)
	{
		if (event->type != ev_keydown)
			return false;
			
		if (keypressed)
			return false;
			
		keypressed = true;
		return true;
	}
	return false;
}

//
// F_Ticker
//
void F_Ticker(void)
{
	// advance animation
	finalecount++;
	
	switch (finalestage)
	{
		case 0:
			// check for skipping
			if (keypressed)
			{
				keypressed = false;
				if ((unsigned)finalecount < strlen(finaletext) * TEXTSPEED)
					// force text to be write
					finalecount += INT_MAX / 2;
				else if (gamemode == commercial)
				{
					if (gamemap == 30)
						F_StartCast();
					else
					{
						gameaction = ga_worlddone;
						finalecount = INT_MIN;	// wait until map is lunched
					}
				}
				else
					// next animation state (just above)
					finalecount = INT_MAX;
			}
			
			if (gamemode != commercial)
			{
				uint32_t f = finalecount;
				
				if (f >= INT_MAX / 2)
					f -= INT_MAX / 2;
					
				if (f > strlen(finaletext) * TEXTSPEED + TEXTWAIT)
				{
					finalecount = 0;
					finalestage = 1;
					wipegamestate = -1;	// force a wipe
					if (gameepisode == 3)
						S_ChangeMusic(mus_bunny, false);
				}
			}
			break;
		case 2:
			F_CastTicker();
			break;
		default:
			break;
	}
}

//
// F_TextWrite
//
void F_TextWrite(void)
{
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
	char* name;
	mobjtype_t type;
} castinfo_t;

castinfo_t castorder[] =
{
#if 0
	{NULL, MT_POSSESSED},
	{NULL, MT_SHOTGUY},
	{NULL, MT_CHAINGUY},
	{NULL, MT_TROOP},
	{NULL, MT_SERGEANT},
	{NULL, MT_SKULL},
	{NULL, MT_HEAD},
	{NULL, MT_KNIGHT},
	{NULL, MT_BRUISER},
	{NULL, MT_BABY},
	{NULL, MT_PAIN},
	{NULL, MT_UNDEAD},
	{NULL, MT_FATSO},
	{NULL, MT_VILE},
	{NULL, MT_SPIDER},
	{NULL, MT_CYBORG},
	{NULL, MT_PLAYER},
#endif
	{NULL, 0}
};

int castnum;
int casttics;
state_t* caststate;
bool_t castdeath;
int castframes;
int castonmelee;
bool_t castattacking;

//
// F_StartCast
//

void F_StartCast(void)
{
#if 0
	int i;
	
	for (i = 0; i < 17; i++)
		castorder[i].name = text[CC_ZOMBIE_NUM + i];
		
	wipegamestate = -1;			// force a screen wipe
	castnum = 0;
	caststate = states[mobjinfo[castorder[castnum].type]->seestate];
	casttics = caststate->tics;
	castdeath = false;
	finalestage = 2;
	castframes = 0;
	castonmelee = 0;
	castattacking = false;
	S_ChangeMusic(mus_evil, true);
#endif
}

//
// F_CastTicker
//
void F_CastTicker(void)
{
	int st;
	int sfx;
	
	if (--casttics > 0)
		return;					// not time to change state yet
		
	if (caststate->tics == -1 || caststate->nextstate == S_NULL)
	{
		// switch from deathstate to next monster
		castnum++;
		castdeath = false;
		if (castorder[castnum].name == NULL)
			castnum = 0;
		if (mobjinfo[castorder[castnum].type]->seesound)
			S_StartSound(NULL, mobjinfo[castorder[castnum].type]->seesound);
		caststate = states[mobjinfo[castorder[castnum].type]->seestate];
		castframes = 0;
	}
	else
	{
		// just advance to next state in animation
		//if (caststate == states[S_PLAY_ATK1])
		//	goto stopattack;	// Oh, gross hack!
		st = caststate->nextstate;
		caststate = states[st];
		castframes++;
		
		// sound hacks....
		switch (st)
		{
#if 0
			case S_PLAY_ATK1:
				sfx = sfx_dshtgn;
				break;
			case S_POSS_ATK2:
				sfx = sfx_pistol;
				break;
			case S_SPOS_ATK2:
				sfx = sfx_shotgn;
				break;
			case S_VILE_ATK2:
				sfx = sfx_vilatk;
				break;
			case S_SKEL_FIST2:
				sfx = sfx_skeswg;
				break;
			case S_SKEL_FIST4:
				sfx = sfx_skepch;
				break;
			case S_SKEL_MISS2:
				sfx = sfx_skeatk;
				break;
			case S_FATT_ATK8:
			case S_FATT_ATK5:
			case S_FATT_ATK2:
				sfx = sfx_firsht;
				break;
			case S_CPOS_ATK2:
			case S_CPOS_ATK3:
			case S_CPOS_ATK4:
				sfx = sfx_shotgn;
				break;
			case S_TROO_ATK3:
				sfx = sfx_claw;
				break;
			case S_SARG_ATK2:
				sfx = sfx_sgtatk;
				break;
			case S_BOSS_ATK2:
			case S_BOS2_ATK2:
			case S_HEAD_ATK2:
				sfx = sfx_firsht;
				break;
			case S_SKULL_ATK2:
				sfx = sfx_sklatk;
				break;
			case S_SPID_ATK2:
			case S_SPID_ATK3:
				sfx = sfx_shotgn;
				break;
			case S_BSPI_ATK2:
				sfx = sfx_plasma;
				break;
			case S_CYBER_ATK2:
			case S_CYBER_ATK4:
			case S_CYBER_ATK6:
				sfx = sfx_rlaunc;
				break;
			case S_PAIN_ATK3:
				sfx = sfx_sklatk;
				break;
#endif
			default:
				sfx = 0;
				break;
		}
		
		if (sfx)
			S_StartSound(NULL, sfx);
	}
	
	if (castframes == 12)
	{
		// go into attack frame
		castattacking = true;
		if (castonmelee)
			caststate = states[mobjinfo[castorder[castnum].type]->meleestate];
		else
			caststate = states[mobjinfo[castorder[castnum].type]->missilestate];
		castonmelee ^= 1;
		if (caststate == states[S_NULL])
		{
			if (castonmelee)
				caststate = states[mobjinfo[castorder[castnum].type]->meleestate];
			else
				caststate = states[mobjinfo[castorder[castnum].type]->missilestate];
		}
	}
	
	if (castattacking)
	{
		if (castframes == 24 || caststate == states[mobjinfo[castorder[castnum].type]->seestate])
		{
stopattack:
			castattacking = false;
			castframes = 0;
			caststate = states[mobjinfo[castorder[castnum].type]->seestate];
		}
	}
	
	casttics = caststate->tics;
	if (casttics == -1)
		casttics = 15;
}

//
// F_CastResponder
//

bool_t F_CastResponder(event_t* ev)
{
	if (ev->type != ev_keydown)
		return false;
		
	if (castdeath)
		return true;			// already in dying frames
		
	// go into death frame
	castdeath = true;
	caststate = states[mobjinfo[castorder[castnum].type]->deathstate];
	casttics = caststate->tics;
	castframes = 0;
	castattacking = false;
	if (mobjinfo[castorder[castnum].type]->deathsound)
		S_StartSound(NULL, mobjinfo[castorder[castnum].type]->deathsound);
		
	return true;
}

#define CASTNAME_Y   180		// where the name of actor is drawn
void F_CastPrint(char* text)
{
	V_DrawStringA(VFONT_SMALL, VFO_CENTERED, text, 0, CASTNAME_Y);
}

//
// F_CastDrawer
//
void F_CastDrawer(void)
{
	spritedef_t* sprdef;
	spriteframe_t* sprframe;
	int lump;
	bool_t flip;
	patch_t* patch;
	
	// erase the entire screen to a background
	//V_DrawPatch (0,0,0, W_CachePatchName ("BOSSBACK", PU_CACHE));
	D_PageDrawer("BOSSBACK");
	
	F_CastPrint(castorder[castnum].name);
	
	// draw the current frame in the middle of the screen
	sprdef = &sprites[caststate->sprite];
	sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];
	lump = sprframe->lumppat[0];	//Fab: see R_InitSprites for more
	flip = (bool_t)sprframe->flip[0];
	
	//patch = W_CachePatchNum(lump, PU_CACHE);
	
	//V_DrawScaledPatch(BASEVIDWIDTH >> 1, 170, flip ? V_FLIPPEDPATCH : 0, patch);
}

//
// F_DrawPatchCol
//
static void F_DrawPatchCol(int x, patch_t* patch, int col)
{
	column_t* column;
	uint8_t* source;
	uint8_t* dest;
	uint8_t* desttop;
	int count;
	
	column = (column_t*) ((uint8_t*)patch + LittleSwapInt32(patch->columnofs[col]));
	desttop = screens[0] + x * vid.dupx;
	
	// step through the posts in a column
	while (column->topdelta != 0xff)
	{
		source = (uint8_t*)column + 3;
		dest = desttop + column->topdelta * vid.width;
		count = column->length;
		
		while (count--)
		{
			int dupycount = vid.dupy;
			
			while (dupycount--)
			{
				int dupxcount = vid.dupx;
				
				while (dupxcount--)
					*dest++ = *source;
					
				dest += (vid.width - vid.dupx);
			}
			source++;
		}
		column = (column_t*) ((uint8_t*)column + column->length + 4);
	}
}

//
// F_BunnyScroll
//
void F_BunnyScroll(void)
{
	int scrolled;
	int x;
	patch_t* p1;
	patch_t* p2;
	char name[10];
	int stage;
	static int laststage;
	
#if 0
	p1 = W_CachePatchName("PFUB2", PU_LEVEL);
	p2 = W_CachePatchName("PFUB1", PU_LEVEL);
#endif
	
	scrolled = 320 - (finalecount - 230) / 2;
	if (scrolled > 320)
		scrolled = 320;
	if (scrolled < 0)
		scrolled = 0;
		
	for (x = 0; x < 320; x++)
	{
		if (x + scrolled < 320)
			F_DrawPatchCol(x, p1, x + scrolled);
		else
			F_DrawPatchCol(x, p2, x + scrolled - 320);
	}
	
	if (finalecount < 1130)
		return;
	if (finalecount < 1180)
	{
		//V_DrawScaledPatch((320 - 13 * 8) / 2, (200 - 8 * 8) / 2, 0, W_CachePatchName("END0", PU_CACHE));
		laststage = 0;
		return;
	}
	
	stage = (finalecount - 1180) / 5;
	if (stage > 6)
		stage = 6;
	if (stage > laststage)
	{
		S_StartSound(NULL, sfx_pistol);
		laststage = stage;
	}
	
	sprintf(name, "END%i", stage);
	//V_DrawScaledPatch((320 - 13 * 8) / 2, (200 - 8 * 8) / 2, 0, W_CachePatchName(name, PU_CACHE));
}

//
// F_Drawer
//
void F_Drawer(void)
{
#if 0
	if (finalestage == 2)
	{
		F_CastDrawer();
		return;
	}
	
	if (!finalestage)
		F_TextWrite();
	else
	{
		switch (gameepisode)
		{
			case 1:
				if (gamemode == retail)
					V_DrawScaledPatch(0, 0, 0, W_CachePatchName(text[CREDIT_NUM], PU_CACHE));
				else
					V_DrawScaledPatch(0, 0, 0, W_CachePatchName(text[HELP2_NUM], PU_CACHE));
				break;
			case 2:
				V_DrawScaledPatch(0, 0, 0, W_CachePatchName(text[VICTORY2_NUM], PU_CACHE));
				break;
			case 3:
				F_BunnyScroll();
				break;
			case 4:
				V_DrawScaledPatch(0, 0, 0, W_CachePatchName(text[ENDPIC_NUM], PU_CACHE));
				break;
		}
	}
#endif
}
