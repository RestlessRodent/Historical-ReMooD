// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

#ifndef __M_RANDOM__
#define __M_RANDOM__

#include "doomtype.h"

// Returns a number from 0 to 255,
// from a lookup table.
uint8_t M_Random(void);

//#define DEBUGRANDOM

#ifdef DEBUGRANDOM
#define P_Random() P_Random2(__FILE__,__LINE__)
#define P_SignedRandom() P_SignedRandom2(__FILE__,__LINE__)
uint8_t P_Random2(char* a, int b);
int P_SignedRandom2(char* a, int b);
#else
// As M_Random, but used only by the play simulation.
uint8_t P_Random(void);
int P_SignedRandom();
#endif

// Fix randoms for demos.
void M_ClearRandom(void);

uint8_t P_GetRandIndex(void);

void P_SetRandIndex(uint8_t rindex);

extern uint8_t* g_RandomData;					// Random Data
extern uint32_t g_RandomDataSize;				// Size of random data

#endif

