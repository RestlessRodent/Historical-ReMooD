// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Global RMOD Parsing

#ifndef __D_RMOD_H__
#define __D_RMOD_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "dstrings.h"
#include "w_wad.h"
#include "z_zone.h"

/****************
*** CONSTANTS ***
****************/

/*****************
*** STRUCTURES ***
*****************/

/*****************
*** PROTOTYPES ***
*****************/

void D_WX_RMODMultiBuild(WX_WADFile_t* const a_WAD, const WX_BuildAction_t a_Action);

void D_WX_RMODBuild(WX_WADFile_t* const a_WAD);
void D_WX_RMODClearBuild(WX_WADFile_t* const a_WAD);
void D_WX_RMODComposite(WX_WADFile_t* const a_WAD);
void D_WX_RMODClearComposite(void);

#endif /* __D_RMOD_H__ */

