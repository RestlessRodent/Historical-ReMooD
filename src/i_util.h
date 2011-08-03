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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@gmail.com>
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
// DESCRIPTION: Common Interface Utilities (to reduce code bloat and dup)

#ifndef __I_UTIL_H__
#define __I_UTIL_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** FUNCTIONS ***
****************/

int VID_NumModes(void);
char* __REMOOD_DEPRECATED VID_GetModeName(int a_ModeNum);
int VID_ClosestMode(int* const a_WidthP, int* const a_HeightP, const boolean a_Fullscreen);
int __REMOOD_DEPRECATED VID_GetModeForSize(int a_Width, int a_Height);
boolean VID_AddMode(const int a_Width, const int a_Height, const boolean a_Fullscreen);
int VID_SetMode(int a_ModeNum);

boolean I_UtilWinArgToUNIXArg(int* const a_argc, char*** const a_argv, const char* const a_Win);
boolean I_VideoPreInit(void);
boolean I_VideoBefore320200Init(void);
boolean I_VideoPostInit(void);
void I_VideoSetBuffer(const uint32_t a_Width, const uint32_t a_Height, const uint32_t a_Pitch, uint8_t* const a_Direct);
void I_VideoUnsetBuffer(void);
uint8_t* I_VideoSoftBuffer(uint32_t* const a_WidthP, uint32_t* const a_HeightP);

#endif /* __I_UTIL_H__ */

