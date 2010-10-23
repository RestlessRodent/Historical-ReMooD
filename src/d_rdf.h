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
// Copyright (C) 2008 ReMooD Team.
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
// DESCRIPTION: ReMooD Demo Format

#ifndef __D_RDF_H__
#define __D_RDF_H__

#include "doomdef.h"
#include "g_game.h"

typedef enum
{
	DST_SUCCESS,
	DST_NOTRECORDING,
	
	MAXDEMOSTATUS
} DemoStatus_t;

/*** GLOBAL FUNCTIONS ***/
DemoStatus_t RDF_DEMO_PrepareRecording(char* name);
DemoStatus_t RDF_DEMO_StartRecording(void);
DemoStatus_t RDF_DEMO_EndRecording(void);

/*** WRITE FUNCTIONS ***/

/*** READ FUNCTIONS ***/

#endif /* __D_RDF_H__ */

