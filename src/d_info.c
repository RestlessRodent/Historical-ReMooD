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
// DESCRIPTION: *INFO processing

/***************
*** INCLUDES ***
***************/

#include "d_info.h"
#include "z_zone.h"

/****************
*** FUNCTIONS ***
****************/

/* D_WXBuildInfos() -- Builds all INFOs */
void D_WXBuildInfos(const D_InfoLoadType_t a_Type, WX_WADFile_t* const a_WAD)
{
	/* What are we doing? */
	switch (a_Type)
	{
			// Load all the INFOs for a single WAD
		case DILT_BUILDONE:
			break;
			
			// Compile all the stuff together
		case DILT_BUILDALL:
			break;
		
			// Nothing
		default:
			break;
	}
}


