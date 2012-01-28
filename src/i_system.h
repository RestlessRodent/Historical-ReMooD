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
// DESCRIPTION: System specific interface stuff.

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include <stdio.h>

#include "doomtype.h"
#include "d_ticcmd.h"
#include "d_event.h"

// See Shutdown_xxx() routines.
extern uint8_t graphics_started;
extern uint8_t keyboard_started;
extern uint8_t sound_started;

//extern uint8_t music_installed;

/* flag for 'win-friendly' mode used by interface code */
extern volatile tic_t ticcount;

//
// Called by D_DoomLoop,
// called before processing any tics in a frame
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

void I_Error(char* error, ...);

// Allocates from low memory under dos,
// just mallocs under unix
uint8_t* I_AllocLow(int length);

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

#ifdef LINUX
void I_LocateWad(void);
#endif

void I_RegisterCrash(void);

/*****************
*** PROTOTYPES ***
*****************/

int I_mkdir(const char* a_Path, int a_UNIXPowers);
void* I_SysAlloc(const size_t a_Size);
void* I_SysRealloc(void* const a_Ptr, const size_t a_NewSize);
void I_SysFree(void* const a_Ptr);
void I_SystemPreExit(void);
void I_SystemPostExit(void);
uint32_t I_GetTimeMS(void);

#endif
