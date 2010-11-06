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
// Constants
#define _FIXED_FRACBITS 16
#define _FIXED_ONE (1 << _FIXED_FRACBITS)
#define _FIXED_TWO (2 << _FIXED_FRACBITS)
#define _FIXED_NEGONE (-1 << _FIXED_FRACBITS)
#define _FIXED_SIGN		0x80000000
#define _FIXED_INT		0xFFFF0000
#define _FIXED_FRAC		0x0000FFFF
#define _FIXED_ROUND	0x00008000

// Compatibility
#define FRACBITS _FIXED_FRACBITS
#define FRACUNIT (1 << _FIXED_FRACBITS)

typedef Int32 fixed_t;
#define FIXED_TO_FLOAT(x) (((float)(x)) / 65536.0)

//
// Declare those functions:
/*
fixed_t FixedMul (fixed_t a, fixed_t b);
fixed_t FixedDiv (fixed_t a, fixed_t b);
fixed_t FixedDiv2 (fixed_t a, fixed_t b);
*/

/* FixedMul() -- Multiply two fixed numbers */
static fixed_t ATTRIB_FORCEINLINE ATTRIB_UNUSED FixedMul(fixed_t a, fixed_t b)
{
	// Copyright (C) 2010 GhostlyDeath (ghostlydeath@gmail.com / ghostlydeath@remood.org)
	register UInt32 w, x, y, z;
	UInt32 A, B;
	fixed_t Res;
	UInt8 Bits;
	
	/* Short circuit */
	// These comparisons may be cheaper!
	if (b == _FIXED_ONE)
		return a;
	else if (b == _FIXED_NEGONE)
		return -a;
	else if (b == 0)
		return 0;
	
	/* Long math */
	else
	{
		// Clear bits
		Bits = 0;
		
		// Get A and B
		if (a & _FIXED_SIGN)
		{
			Bits |= 1;
			A = -a;
		}
		else
			A = a;
			
		if (b & _FIXED_SIGN)
		{
			Bits |= 2;
			B = -b;
		}
		else
			B = b;
		
		// Multiply sections of the numbers
		w = ((A & _FIXED_FRAC) * (B & _FIXED_FRAC)) >> _FIXED_FRACBITS;
		x = ((A & _FIXED_INT) >> _FIXED_FRACBITS) * (B & _FIXED_FRAC);
		y = (A & _FIXED_FRAC) * ((B & _FIXED_INT) >> _FIXED_FRACBITS);
		z = (((A & _FIXED_INT) >> _FIXED_FRACBITS) * ((B & _FIXED_INT) >> _FIXED_FRACBITS)) << _FIXED_FRACBITS;
		
		// Get result
		Res = (w + x + y + z);
		
		// Return additions
		if (Bits == 2 || Bits == 1)
			Res = -(Res);	// + * - or - * +
		
		return Res;
	}
}

/* FixedPtInv() -- Inverse of fixed point */
static fixed_t ATTRIB_FORCEINLINE ATTRIB_UNUSED FixedInv(const fixed_t a)
{
	// Copyright (C) 2010 GhostlyDeath (ghostlydeath@gmail.com / ghostlydeath@remood.org)
	register fixed_t A, SDiv, Res;
	
	/* Short circuit */
	// These comparisons may be cheaper!
	if (a == _FIXED_ONE)
		return _FIXED_ONE;
	
	/* Long math */
	else
	{
		// Set A from a
		A = a;
		if (A & _FIXED_SIGN)
			A = -A;
		
		// Set division a bit smaller
		SDiv = A >> 2;
		
		// If division is big enough and not zero
		if (SDiv)
			Res = ((Int32)1 << ((_FIXED_FRACBITS << (Int32)1) - 2)) / SDiv;
		
		// A is a small number
		else if (A == 3)
			Res = 0x55550806;
		
		// A is super small or zero
		else
			Res = 0x7FFFFFFF;
		
		// If a was originally negative, return negative result
		if (a & _FIXED_SIGN)
			return -Res;
		
		// Otherwise it's positive
		else
			return Res;
	}
}

/* FixedDiv() -- Divide two fixed numbers */
static fixed_t ATTRIB_FORCEINLINE ATTRIB_UNUSED FixedDiv(fixed_t a, fixed_t b)
{
	// Copyright (C) 2010 GhostlyDeath (ghostlydeath@gmail.com / ghostlydeath@remood.org)
	if (a == b)
		return _FIXED_ONE;	// TODO -- Breaks vanilla
	else
		return FixedMul(a, FixedInv(b));
}

#if 0
fixed_t FixedDiv2(fixed_t a, fixed_t b);

static inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	//I_Error("<a: %ld, b: %ld>",(long)a,(long)b);

	if ((abs(a) >> 14) >= abs(b))
		return (a ^ b) < 0 ? MININT : MAXINT;

	return FixedDiv2(a, b);
}
#endif

#endif							/* __M_FIXED_H__ */
