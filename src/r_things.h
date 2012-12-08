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
// Copyright (C) 2008-2013 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Rendering of moving objects, sprites.

#ifndef __R_THINGS__
#define __R_THINGS__

#include "sounds.h"

// GhostlyDeath <July 24, 2011> -- Remove sprite limit
#define NUMSPRITEBUMPS	512

#define MAXVISSPRITES   512		// added 2-2-98 was 128

// Constant arrays used for psprite clipping
//  and initializing clipping.
extern short* negonearray;
extern short* screenheightarray;

// vars for R_DrawMaskedColumn
extern short* mfloorclip;
extern short* mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t sprbotscreen;
extern fixed_t windowtop;
extern fixed_t windowbottom;

extern fixed_t pspritescale;
extern fixed_t pspriteiscale;
extern fixed_t pspriteyscale;	//added:02-02-98:for aspect ratio

extern const int PSpriteSY[];

void R_DrawMaskedColumn(column_t* column);

void R_SortVisSprites(void);

//faB: find sprites in wadfile, replace existing, add new ones
//     (only sprites from namelist are added or replaced)
void R_AddSpriteDefs(char** namelist, int wadnum);

//SoM: 6/5/2000: Light sprites correctly!
void R_AddSprites(sector_t* sec, int lightlevel);
void R_AddPSprites(void);
void R_DrawSprite(vissprite_t* spr);
void R_InitSprites(char** namelist);
void R_ClearSprites(void);
void R_DrawSprites(void);		//draw all vissprites
void R_DrawMasked(void);

void R_ClipVisSprite(vissprite_t* vis, int xl, int xh);

// -----------
// SKINS STUFF
// -----------
#define SKINNAMESIZE 16
#define DEFAULTSKIN  "marine"	// name of the standard doom marine as skin

typedef struct
{
	char name[SKINNAMESIZE + 1];	// short descriptive name of the skin
	spritedef_t spritedef;
	char faceprefix[4];			// 3 chars+'\0', default is "STF"
	
	// specific sounds per skin
	short soundsid[NUMSKINSOUNDS];	// sound # in S_sfx table
	
} skin_t;

extern int numskins;
extern skin_t* skins;

//void    R_InitSkins (void);
void SetPlayerSkin(int playernum, char* skinname);
int R_SkinAvailable(char* name);
void R_AddSkins(int wadnum);

void R_InitDrawNodes();

#endif /*__R_THINGS__*/
