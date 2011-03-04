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
// Copyright(C) 2000 Simon Howard
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
// DESCRIPTION: FraggleScript files...

// #############################################################################
// ##  THIS SOURCE FILE HAS BEEN DEPRECATED AND WILL BE REMOVED IN THE FUTURE ##
// #############################################################################
// # There should be no futher changes unless necessary. Future dependencies   #
// # on this code will be changed, replaced and/or removed.                    #
// #############################################################################
// # NOTE: Deprecated and will be replaced by the new ReMooD Virtual Machine   #
// #       Which will be the heart of ReMooD Script. Of course there is a      #
// #       Legacy compatibility layer that will be maintained so all those     #
// #       awesome Legacy mods can be played.                                  #
// #############################################################################

#ifndef __FUNC_H__
#define __FUNC_H__

#include "t_parse.h"

#include "p_local.h"

extern camera_t script_camera;
extern boolean script_camera_on;

void init_functions();

#define AngleToFixed(x)  (((double) x) / ((double) ANG45/45)) * FRACUNIT
#define FixedToAngle(x)  (((double) x) / FRACUNIT) * ANG45/45;

#endif
