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
// DESCRIPTION: host/client network commands
//              commands are executed through the command buffer
//              like console commands

#ifndef __D_NETCMD__
#define __D_NETCMD__

#include "command.h"

// console vars
extern consvar_t cv_playername;
extern consvar_t cv_playercolor;
extern consvar_t cv_use_mouse;
extern consvar_t cv_use_joystick;
extern consvar_t cv_autoaim;
extern consvar_t cv_controlperkey;

// splitscreen with seconde mouse
extern consvar_t cv_mouse2port;
extern consvar_t cv_use_mouse2;
#ifdef LMOUSE2
extern consvar_t cv_mouse2opt;
#endif
extern consvar_t cv_invertmouse2;
extern consvar_t cv_alwaysfreelook2;
extern consvar_t cv_mousemove2;
extern consvar_t cv_mousesens2;
extern consvar_t cv_mlooksens2;

// normaly in p_mobj but the .h in not read !
extern consvar_t cv_itemrespawntime;
extern consvar_t cv_itemrespawn;
extern consvar_t cv_respawnmonsters;
extern consvar_t cv_respawnmonsterstime;

// added 16-6-98 : splitscreen
extern consvar_t cv_splitscreen;

// 02-08-98      : r_things.c
extern consvar_t cv_skin;

// secondary splitscreen player
extern consvar_t cv_playername2;
extern consvar_t cv_playercolor2;
extern consvar_t cv_skin2;

extern consvar_t cv_teamplay;
extern consvar_t cv_teamdamage;
extern consvar_t cv_fraglimit;
extern consvar_t cv_timelimit;
extern uint32_t timelimitintics;
extern consvar_t cv_allowturbo;
extern consvar_t cv_allowexitlevel;

extern consvar_t cv_netstat;
extern consvar_t cv_translucency;
extern consvar_t cv_splats;
extern consvar_t cv_maxsplats;
extern consvar_t cv_screenslink;

// add game commands, needs cleanup
void D_RegisterClientCommands(void);
void D_SendPlayerConfig(void);
void Command_ExitGame_f(void);

extern CV_PossibleValue_t fraglimit_cons_t[];
extern CV_PossibleValue_t teamplay_cons_t[];
extern CV_PossibleValue_t deathmatch_cons_t[];

#endif

