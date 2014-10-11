// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: ReMooD Scripting Virtual Machine

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "w_wad.h"

/*****************
*** STRUCTURES ***
*****************/

/*****************
*** PROTOTYPES ***
*****************/

bool_t T_DSVM_Cleanup(void);
bool_t T_DSVM_CompileStream(WL_ES_t* const a_Stream, const size_t a_End);

