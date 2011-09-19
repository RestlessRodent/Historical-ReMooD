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
// DESCRIPTION:
//      Demo Compatibility

#ifndef __P_DEMCMP_H__
#define __P_DEMCMP_H__

#include "console.h"
#include "doomdef.h"
#include "doomstat.h"

/* Demo Compatibility Functions */
void DC_RegisterDemoCompVars(void);
void DC_SetMenuGameOptions(int SetDemo);
void DC_SetDemoOptions(int VerToSet);

/* Instead of tons of if (demoplayback) blah, do this instead, alot easier */
#define ___DCMERGE(a,b) a##b
#define DEMOCVAR(a) (demoplayback ? ___DCMERGE(cv_dc_,a) : ___DCMERGE(cv_,a))

/* Demo Compatibility CVARs */
extern consvar_t cv_dc_allowjump;
extern consvar_t cv_dc_allowautoaim;
extern consvar_t cv_dc_forceautoaim;
extern consvar_t cv_dc_allowrocketjump;
extern consvar_t cv_dc_classicblood;
extern consvar_t cv_dc_predictingmonsters;
extern consvar_t cv_dc_classicrocketblast;
extern consvar_t cv_dc_classicmeleerange;
extern consvar_t cv_dc_classicmonsterlogic;

#endif							/* __P_DEMCMP_H__ */
