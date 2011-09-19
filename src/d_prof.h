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
// DESCRIPTION: Profiles

#ifndef __D_PROF_H__
#define __D_PROF_H__

#include "doomtype.h"
#include "doomdef.h"
#include "command.h"

enum
{
	PC_NAME,
	PC_COLOR,
	PC_SKIN,
	PC_AUTOAIM,
	
	MAXPROFILECVARS
};

typedef struct ProfileInfo_s
{
	char name[MAXPLAYERNAME];
	consvar_t cvars[MAXPROFILECVARS];
	
	struct ProfileInfo_s* prev;
	struct ProfileInfo_s* next;
} ProfileInfo_t;

extern ProfileInfo_t NonLocalProfile;

void PROF_Init(void);
void PROF_Shutdown(void);
void PROF_HandleVAR(char* arg0, char* arg1);

void M_StartProfiler(int choice);
void M_ProfilePrompt(int player);

#endif							/* __D_PROF_H__ */
