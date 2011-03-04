// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: Sky rendering.

#ifndef __R_SKY__
#define __R_SKY__

#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

// SKY, store the number for name.
#define                 SKYFLATNAME  "F_SKY1"

// The sky map is 256*128*4 maps.
#define ANGLETOSKYSHIFT         22

extern int skytexture;
extern int skytexturemid;
extern fixed_t skyscale;
extern int skymode;				//current sky old (0) or new(1),
								  // see SCR_SetMode

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.
extern int skyflatnum;

//added:12-02-98: declare the asm routine which draws the sky columns
void R_DrawSkyColumn(void);

// Called once at startup.
void R_InitSkyMap(void);

// call after skytexture is set to adapt for old/new skies
void R_SetupSkyDraw(void);

void R_StorePortalRange(void);
void R_InitPortals();
void R_ClearPortals();
void R_DrawPortals();

void R_SetSkyScale(void);

#endif
