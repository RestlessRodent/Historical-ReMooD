// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION: System specific interface stuff.

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

// Takes full 8 bit values.
void I_SetPalette(RGBA_t* palette);

void I_UpdateNoBlit(void);
void I_FinishUpdate(void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen(uint8_t* scr);

/****************
*** FUNCTIONS ***
****************/

void VID_PrepareModeList(void);
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen);
void I_StartupGraphics(void);
void I_ShutdownGraphics(void);
bool_t I_TextMode(const bool_t a_OnOff);
void I_VideoLockBuffer(const bool_t a_DoLock);

size_t I_ProbeJoysticks(void);
void I_RemoveJoysticks(void);
bool_t I_GetJoystickID(const size_t a_JoyID, uint32_t* const a_Code, char* const a_Text, const size_t a_TextSize, char* const a_Cool, const size_t a_CoolSize);
bool_t I_GetJoystickCounts(const size_t a_JoyID, uint32_t* const a_NumAxis, uint32_t* const a_NumButtons);

bool_t I_ProbeMouse(const size_t a_ID);
bool_t I_RemoveMouse(const size_t a_ID);
void I_MouseGrab(const bool_t a_Grab);

#endif
