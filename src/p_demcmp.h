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

/*****************************
*** EXTENDED GAME SETTINGS ***
*****************************/

// The way settings were done in Legacy were that they were console variables,
// so you would have tons of console variables for every concievable setting.
// However, of all the settings, virtually all of them are either on/off, pure
// integers, floating point numbers, or a specific list of options.
// Doing individually split game settings allows for them to be saved all at
// once and sent by the server all at once, rather than polluting the console
// code with variables splattered all over the place. Game options aren't
// exactly saveable in configs, only the settings that would take effect the
// next game that is played. Also, settings will change drastically when demos
// are played, so you don't want the ugly demoversion checks and you also don't
// want to lost all your game settings when you play a demo or use similar
// settings from the last demo played.

// So Legacy settings are all demoversion and cvars, an ugly mix.

/* P_EXGSType_t -- Setting type for said setting */
typedef enum P_EXGSType_e
{
	PEXGST_INTEGER,								// Integer
	PEXGST_FLOAT,								// Floating Point
	
	NUMPEXGSTYPES
} P_EXGSType_t;

#endif							/* __P_DEMCMP_H__ */

