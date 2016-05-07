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

uint8_t M_Random(void);
uint8_t P_Random(void);
int P_SignedRandom(void);
void M_ClearRandom(void);
uint8_t P_GetRandIndex(void);
void P_SetRandIndex(uint8_t rindex);

#endif

