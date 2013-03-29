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
// DESCRIPTION: Sprite animation.

#ifndef __P_PSPR__
#define __P_PSPR__

// Basic data types.
// Needs fixed point, and BAM angles.
#include "m_fixed.h"
#include "tables.h"
#include "v_video.h"

//
// Needs to include the precompiled
//  sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
#include "info.h"

#include "d_rmod.h"

//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//

// faB: I noticed they didn't use the 32 bits of the frame field,
//      so now we use the upper 16 bits for new effects.

#define FF_FRAMEMASK    0x7fff	// only the frame number
#define FF_FULLBRIGHT   0x8000	// frame always appear full bright (fixedcolormap)

// faB:
//  MF_SHADOW is no more used to activate translucency (or the old fuzzy)
//  The frame field allows to set translucency per frame, instead of per sprite.
//  Now, (frame & FF_TRANSMASK) is the translucency table number, if 0
//  it is not translucent.

// Note:
//  MF_SHADOW still affects the targeting for monsters (they miss more)

#define FF_TRANSMASK   0x0F0000	// 0 = no trans(opaque), 1-7 = transl. table
#define FF_TRANSSHIFT       16

// faB: new 'alpha' shade effect, for smoke..

#define FF_SMOKESHADE  0x800000	// sprite is an alpha channel

// translucency tables

// TODO: add another asm routine which use the fg and bg indexes in the
//       inverse order so the 20-80 becomes 80-20 translucency, no need
//       for other tables (thus 1090,2080,5050,8020,9010, and fire special)
typedef enum
{
	tr_transmed = VEX_TRANS50,	//sprite 50 backg 50  most shots
	tr_transmor = VEX_TRANS80,	//       20       80  puffs
	tr_transhi = VEX_TRANS90,	//       10       90  blur effect
	tr_transfir = VEX_TRANSFIRE,	// 50 50 but brighter for fireballs, shots..
	tr_transfx1 = VEX_TRANSFX1,	// 50 50 brighter some colors, else opaque for torches
} transnum_t;

//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
	ps_weapon,
	ps_flash,
	NUMPSPRITES
} psprnum_t;

typedef struct
{
	PI_state_t* state;				// a NULL state means not active
	int32_t tics;
	fixed_t sx;
	fixed_t sy;
} pspdef_t;

void P_OpenWeapons(void);
void P_CloseWeapons(void);

#endif
