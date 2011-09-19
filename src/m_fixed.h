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

typedef int32_t fixed_t;

#define FIXED_TO_FLOAT(x) (((float)(x)) / 65536.0)

//#define FIXEDBREAKVANILLA

/* FixedRound() -- Round a fixed point number */
static fixed_t __REMOOD_FORCEINLINE __REMOOD_UNUSED FixedRound(const fixed_t a)
{
	/* Negative */
	if (a & _FIXED_SIGN)
		if (!(a & _FIXED_ROUND))
			return (a & _FIXED_INT) - 1;
		else
			return (a & _FIXED_INT);
			
	/* Positive */
	else if (a & _FIXED_ROUND)
		return (a & _FIXED_INT) + 1;
	else
		return (a & _FIXED_INT);
}

/* FixedMul() -- Multiply two fixed numbers */
static fixed_t __REMOOD_FORCEINLINE __REMOOD_UNUSED FixedMul(fixed_t a, fixed_t b)
{
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
	return ((int64_t)a * (int64_t)b) >> _FIXED_FRACBITS;
	
#else
	// Copyright (C) 2010 GhostlyDeath (ghostlydeath@gmail.com / ghostlydeath@remood.org)
	register uint32_t w, x, y, z;
	register uint32_t Af, Ai, Bf, Bi;
	
	if (a & _FIXED_SIGN)
	{
		Af = ((-a) & _FIXED_FRAC);
		Ai = ((-a) & _FIXED_INT) >> _FIXED_FRACBITS;
	}
	else
	{
		Af = (a & _FIXED_FRAC);
		Ai = (a & _FIXED_INT) >> _FIXED_FRACBITS;
	}
	
	if (b & _FIXED_SIGN)
	{
		Bf = ((-b) & _FIXED_FRAC);
		Bi = ((-b) & _FIXED_INT) >> _FIXED_FRACBITS;
	}
	else
	{
		Bf = (b & _FIXED_FRAC);
		Bi = (b & _FIXED_INT) >> _FIXED_FRACBITS;
	}
	
	// Multiply portions
	w = (Af * Bf) >> _FIXED_FRACBITS;
	x = Ai * Bf;
	y = Af * Bi;
	z = (Ai * Bi) << _FIXED_FRACBITS;
	
	// Return result
	if ((a ^ b) & _FIXED_SIGN)	// Only negative result if pos/neg
		return -((int32_t)(w + x + y + z));
	return (w + x + y + z);
#endif
}

/* FixedPtInv() -- Inverse of fixed point */
static fixed_t __REMOOD_FORCEINLINE __REMOOD_UNUSED FixedInv(const fixed_t a)
{
	// Copyright (C) 2010 GhostlyDeath (ghostlydeath@gmail.com / ghostlydeath@remood.org)
	register uint32_t A, SDiv, Res;
	
	/* Short circuit */
	// These comparisons may be cheaper!
	if (a == _FIXED_ONE)
		return _FIXED_ONE;
		
	/* Long math */
	else
	{
		// Set A from a
		if (a & _FIXED_SIGN)
			A = -a;
		else
			A = a;
			
#if 0
		// 16.16 (x/65536 .. 0xFFFF) -> 15.15 (x/32768 .. 0x7FFF)
		// 32,768 = 0x8000 (16) = 0x4000 (15)
		Res = 0x1000U >> 1;
		SDiv = (A >> 1) & 0x7FFFFFFFU;
		
		// Wide multiply like in 64-bit
		Res = (Res << 15U) / SDiv;
		
		// 15.15 -> 16.16
		Res <<= 1;
#else
		// Set division a bit smaller
		SDiv = A >> 1;
		//SDiv = A >> 2;
		
		// If division is big enough and not zero
		if (SDiv)
			Res = ((uint32_t)1 << ((_FIXED_FRACBITS << (uint32_t)1) - 1)) / SDiv;
		
		// A is a small number
		else if (A == 3)
			Res = 0x55550806;
		
		// A is super small or zero
		else
			Res = 0x7FFFFFFF;
#endif
		
		// If a was originally negative, return negative result
		if (a & _FIXED_SIGN)
			return -((fixed_t) Res);
			
		// Otherwise it's positive
		else
			return (fixed_t) Res;
	}
}

/* FixedDiv() -- Divide two fixed numbers */
static fixed_t __REMOOD_FORCEINLINE __REMOOD_UNUSED FixedDiv(fixed_t a, fixed_t b)
{
	// Copyright (C) 2010 GhostlyDeath (ghostlydeath@gmail.com / ghostlydeath@remood.org)
#ifdef FIXEDBREAKVANILLA
	if (a == b)
		return _FIXED_ONE;
	else
		return FixedMul(a, FixedInv(b));	// TODO -- Breaks vanilla
#else
	if (b == 0)
		return 0x7FFFFFFF | (a & 0x80000000);
	else
		return (((int64_t)a) << (int64_t)FRACBITS) / ((int64_t)b);
#endif
}

/* FixedMulSlow() -- Multiply two fixed numbers (slowly) */
static fixed_t __REMOOD_FORCEINLINE __REMOOD_UNUSED FixedMulSlow(fixed_t a, fixed_t b)
{
	return ((int64_t)a * (int64_t)b) >> FRACBITS;
}

/* FixedDivSlow() -- Divide two fixed numbers (slowly) */
static fixed_t __REMOOD_FORCEINLINE __REMOOD_UNUSED FixedDivSlow(fixed_t a, fixed_t b)
{
	return (((int64_t)a) << (int64_t)FRACBITS) / ((int64_t)b);
}

#endif							/* __M_FIXED_H__ */
