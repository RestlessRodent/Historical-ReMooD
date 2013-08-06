// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
//                                      <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: INI-like parsing

#ifndef __T_INI_H__
#define __T_INI_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
//#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

#define MAXSECTIONAME						48	// Size limitation of section names

/*****************
*** STRUCTURES ***
*****************/

/* TINI_Section_t -- INI Section */
typedef struct TINI_Section_s
{
	WL_ES_t* Stream;							// Associated Stream
	
	char Name[MAXSECTIONAME];					// Name of INI Section
	
	uint32_t Start;								// Start of data
	uint32_t End;								// End of data
	
	struct TINI_Section_s* Prev;				// Previous Link
	struct TINI_Section_s* Next;				// Next Link
} TINI_Section_t;

typedef struct TINI_ConfigLine_s TINI_ConfigLine_t;

/****************
*** FUNCTIONS ***
****************/

TINI_Section_t* TINI_FindNextSection(TINI_Section_t* const a_Last, WL_ES_t* const a_Stream);
void TINI_ClearSections(TINI_Section_t* const a_Iter);

TINI_ConfigLine_t* TINI_BeginRead(TINI_Section_t* const a_Section);
void TINI_EndRead(TINI_ConfigLine_t* const a_Parms);
bool_t TINI_ReadLine(TINI_ConfigLine_t* const a_Parms, char** a_Option, char** a_Value);

#endif /* __T_INI_H__ */

