// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: System specific interface stuff.

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "doomtype.h"



// See Shutdown_xxx() routines.
extern uint8_t graphics_started;
extern uint8_t keyboard_started;
extern uint8_t sound_started;

//extern uint8_t music_installed;

/* flag for 'win-friendly' mode used by interface code */
extern volatile tic_t ticcount;

//
// Called by D_DoomLoop,
// called before processiI_LocateWadng any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
void I_StartFrame(void);

//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_OsPolling(void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Called by M_Responder when quit is selected, return code 0.
void I_Quit(void);

void I_Tactile(int on, int off, int total);

//added:18-02-98: write a message to stderr (use before I_Quit)
//                for when you need to quit with a msg, but need
//                the return code 0 of I_Quit();
void I_OutputMsg(char* error, ...);
void I_OutputText(const char* const a_Text);

// keyboard startup,shutdown,handler
void I_StartupKeyboard(void);

void I_UpdateJoysticks(void);

// setup timer irq and user timer routine.
void I_TimerISR(void);			//timer callback routine.
void I_StartupTimer(void);

// Setup signal handler, plus stuff for trapping errors and cleanly exit.
int I_StartupSystem(void);

void I_RegisterCrash(void);

/*****************
*** PROTOTYPES ***
*****************/

void* I_SysAlloc(const size_t a_Size);
void* I_SysRealloc(void* const a_Ptr, const size_t a_NewSize);
void I_SysFree(void* const a_Ptr);
void I_SystemPreExit(void);
void I_SystemPostExit(void);
uint32_t I_GetTimeMS(void);

#endif
