// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
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

// number of sprite lumps for spritewidth,offset,topoffset lookup tables
// Fab: this is a hack : should allocate the lookup tables per sprite
#define     MAXSPRITELUMPS     4096

#define MAXVISSPRITES   256		// added 2-2-98 was 128

// Constant arrays used for psprite clipping
//  and initializing clipping.
extern short* negonearray;
extern short* screenheightarray;

// vars for R_DrawMaskedColumn
extern short *mfloorclip;
extern short *mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t sprbotscreen;
extern fixed_t windowtop;
extern fixed_t windowbottom;

extern fixed_t pspritescale;
extern fixed_t pspriteiscale;
extern fixed_t pspriteyscale;	//added:02-02-98:for aspect ratio

extern const int PSpriteSY[];

void R_DrawMaskedColumn(column_t * column);

void R_SortVisSprites(void);

//faB: find sprites in wadfile, replace existing, add new ones
//     (only sprites from namelist are added or replaced)
void R_AddSpriteDefs(char **namelist, int wadnum);

//SoM: 6/5/2000: Light sprites correctly!
void R_AddSprites(sector_t * sec, int lightlevel);
void R_AddPSprites(void);
void R_DrawSprite(vissprite_t * spr);
void R_InitSprites(char **namelist);
void R_ClearSprites(void);
void R_DrawSprites(void);		//draw all vissprites
void R_DrawMasked(void);

void R_ClipVisSprite(vissprite_t * vis, int xl, int xh);

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
extern skin_t skins[MAXSKINS + 1];
//extern CV_PossibleValue_t skin_cons_t[MAXSKINS+1];
extern consvar_t cv_skin;

//void    R_InitSkins (void);
void SetPlayerSkin(int playernum, char *skinname);
int R_SkinAvailable(char *name);
void R_AddSkins(int wadnum);

void R_InitDrawNodes();

#endif /*__R_THINGS__*/
