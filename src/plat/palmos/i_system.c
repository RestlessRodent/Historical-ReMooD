// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION:

/***************
*** INCLUDES ***
***************/

/* System */
#include "doomdef.h"
#include "m_misc.h"
#include "i_system.h"


#include "g_game.h"

#include "i_video.h"
#include "i_sound.h"
//
//I_OutputMsg
//
void I_OutputMsg(char* fmt, ...)
{
}

/* I_OutputText() -- Output text to console */
void I_OutputText(const char* const a_Text)
{
}

void I_StartupKeyboard(void)
{
}

/* I_StartupTimer() -- Timer startup */
void I_StartupTimer(void)
{
}

int I_GetKey(void)
{
	return 0;
}

void I_JoyScale()
{
}

void I_GetJoyEvent()
{
}

//
// I_Tactile
//
void I_Tactile(int on, int off, int total)
{
	// UNUSED.
	on = off = total = 0;
}

/* I_GetTimeMS() -- Returns time since the game started (in MS) */
uint32_t I_GetTimeMS(void)
{
	return 0;
}

//
// I_Init
//
void I_Init(void)
{
}

/* I_WaitVBL() -- Wait for vertical blank */
void I_WaitVBL(int count)
{
}

//
// I_Error
//
void I_Error(char* error, ...)
{
	// Shutdown. Here might be other errors.
	if (demorecording)
		G_CheckDemoStatus();
		
	// shutdown everything else which was registered
	I_ShutdownSystem();
	
	exit(-1);
}

/****************
*** FUNCTIONS ***
****************/

/* I_SysAlloc() -- Allocate system memory */
void* I_SysAlloc(const size_t a_Size)
{
	return malloc(a_Size);
}

/* I_SysRealloc() -- Reallocate system memory */
void* I_SysRealloc(void* const a_Ptr, const size_t a_NewSize)
{
	return realloc(a_Ptr, a_NewSize);
}

/* I_SysFree() -- Free memory */
void I_SysFree(void* const a_Ptr)
{
	free(a_Ptr);
}

/* I_SystemPreExit() -- Called before functions are exited */
void I_SystemPreExit(void)
{
}

/* I_SystemPostExit() -- Called after functions are exited */
void I_SystemPostExit(void)
{
}
