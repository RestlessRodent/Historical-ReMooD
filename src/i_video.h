// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
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

#define I_VIDEOGLMODECONST 5

void VID_PrepareModeList(void);
bool_t I_SetVideoMode(const uint32_t a_Width, const uint32_t a_Height, const bool_t a_Fullscreen, const uint8_t a_Depth);
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

