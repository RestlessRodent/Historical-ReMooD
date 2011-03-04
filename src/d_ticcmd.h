// Emacs style mode select   -*- C++ -*- 
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
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
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
// DESCRIPTION: System specific interface stuff.

#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "m_fixed.h"
#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

//
// Button/action code definitions.
//

//added:16-02-98: bit of value 64 doesnt seem to be used,
//                now its used to jump

typedef enum
{
	// Press "Fire".
	BT_ATTACK = 1,
	// Use button, to open doors, activate switches.
	BT_USE = 2,

	// Flag, weapon change pending.
	// If true, the next 3 bits hold weapon num.
	BT_CHANGE = 4,
	
	// Jump button.
	BT_JUMP = 8,
	
	// The 3bit weapon mask and shift, convenience.
	BT_WEAPONMASK = (32 + 64 + 128 + 256),
	BT_WEAPONSHIFT = 5,
	BT_EXTRAWEAPON = 512,
	BT_SLOTMASK = (1024 + 2048 + 4096 + 8192),
	BT_SLOTSHIFT = 11,
} buttoncode_t;

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

// bits in angleturn
#define TICCMD_RECEIVED 1
#define TICCMD_XY       2
#define BT_FLYDOWN      4
typedef struct
{
	char forwardmove;			// *2048 for move
	char sidemove;				// *2048 for move
	short angleturn;			// <<16 for angle delta
	// SAVED AS A BYTE into demos
	signed short aiming;		//added:16-02-98:mouse aiming, see G_BuildTicCmd
	uint16_t buttons;
	byte artifact;				// For Heretic
} ticcmd_t;

#endif
