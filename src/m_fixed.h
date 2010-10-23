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
// DESCRIPTION: Fixed point arithemtics, implementation.

#ifndef __M_FIXED__
#define __M_FIXED__

#include "doomtype.h"

//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS                16
#define FRACUNIT                (1<<FRACBITS)
typedef Int32 fixed_t;
#define FIXED_TO_FLOAT(x) (((float)(x)) / 65536.0)

//
// Declare those functions:
/*
fixed_t FixedMul (fixed_t a, fixed_t b);
fixed_t FixedDiv (fixed_t a, fixed_t b);
fixed_t FixedDiv2 (fixed_t a, fixed_t b);
*/

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv2(fixed_t a, fixed_t b);

static inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	//I_Error("<a: %ld, b: %ld>",(long)a,(long)b);

	if ((abs(a) >> 14) >= abs(b))
		return (a ^ b) < 0 ? MININT : MAXINT;

	return FixedDiv2(a, b);
}

#endif							/* __M_FIXED_H__ */
