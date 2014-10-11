// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Simplistic Line Handling Code

#ifndef __P_NEWLINE_H__
#define __P_NEWLINE_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "r_defs.h"
#include "p_spec.h"

/****************
*** CONSTANTS ***
****************/

/* P_NLExtraProp_t -- Extra properties of lines (flags) */
typedef enum P_NLExtraProp_s
{
	PNLXP_DOORLIGHT		= UINT32_C(0x000001),	// Has manual door light
} P_NLExtraProp_t;

/**************
*** STRUCTS ***
**************/

typedef bool_t (*P_NLTrigFunc_t)(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain, const uint32_t a_ArgC, const int32_t* const a_ArgV);

/****************
*** FUNCTIONS ***
****************/

bool_t P_NLTrigger(line_t* const a_Line, const int a_Side, mobj_t* const a_Object, const EV_TryGenType_t a_Type, const uint32_t a_Flags, bool_t* const a_UseAgain);
void P_NLCreateStartLines(void);

uint32_t P_NLSpecialXProp(line_t* const a_Line);

int32_t P_NLDefDoorCloseSnd(void);

#endif /* __P_NEWLINE_H__ */

