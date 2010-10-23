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
// DESCRIPTION: game startup, and main loop code, system specific interface stuff.

#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include "w_wad.h"				// for MAX_WADFILES

//void D_AddFile (char *file);

// make sure not to write back the config until it's been correctly loaded
extern tic_t rendergametic;

// for dedicated server
extern boolean dedicated;

// the infinite loop of D_DoomLoop() called from win_main for windows version
void D_DoomLoop(void);

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
// If not overrided by user input, calls N_AdvanceDemo.
//
void D_DoomMain(void);

// Called by IO functions when input is detected.
void D_PostEvent(const event_t * ev);
void D_PostEvent_end(void);		// delimiter for locking memory

void D_ProcessEvents(void);
void D_DoAdvanceDemo(void);

//
// BASE LEVEL
//
void D_PageTicker(void);
// pagename is lumpname of a 320x200 patch to fill the screen
void D_PageDrawer(char *pagename);
void D_AdvanceDemo(void);
void D_StartTitle(void);

#endif							//__D_MAIN__

