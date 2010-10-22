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
// DESCRIPTION:
//      Fixed point implementation.

#include "i_system.h"
#include "m_fixed.h"

// Fixme. __USE_C_FIXED__ or something.
#ifdef _WIN32
#pragma warning (disable : 4244)
#endif
fixed_t FixedMul(fixed_t a, fixed_t b)
{
	return ((INT64) a * (INT64) b) >> FRACBITS;
}
fixed_t FixedDiv2(fixed_t a, fixed_t b)
{
#if 0
	INT64 c;
	c = ((INT64) a << 16) / ((INT64) b);
	return (fixed_t) c;
#endif

	double c;

	c = ((double)a) / ((double)b) * FRACUNIT;

	if (c >= 2147483648.0 || c < -2147483648.0)
		I_Error("FixedDiv: divide by zero");
	return (fixed_t) c;
}

/*
//
// FixedDiv, C version.
//
fixed_t FixedDiv ( fixed_t   a, fixed_t    b )
{
    //I_Error("<a: %ld, b: %ld>",(long)a,(long)b);

    if ( (abs(a)>>14) >= abs(b))
        return (a^b)<0 ? MININT : MAXINT;

    return FixedDiv2 (a,b);
}
*/
