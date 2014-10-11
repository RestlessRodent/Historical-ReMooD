// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: 32-bit ("True" Color) Drawers

/* NOT IN DEDICATED SERVER */
#if !defined(__REMOOD_DEDICATED)
/***************************/

/***************
*** INCLUDES ***
***************/

#include "ui.h"
#include "ui_dloc.h"

/****************
*** FUNCTIONS ***
****************/

typedef uint32_t ui_pixel_t;

#define UIDG_DEPTH 4
#define UIDG_CODE d32

#include "ui_dgen.h"

/* NOT IN DEDICATED SERVER */
#endif
/***************************/

