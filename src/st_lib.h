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
// DESCRIPTION: The status bar widget code.

#ifndef __STLIB__
#define __STLIB__

// We are referring to patches.
#include "r_defs.h"
#include "w_wad.h"

//
// Background and foreground screen numbers
//
#define BG 1
#define FG 0

//
// Typedefs of widgets
//

// Number widget

typedef struct
{
	// upper right-hand corner
	//  of the number (right-justified)
	int x;
	int y;
	
	// max # of digits in number
	int width;
	
	// last number value
	int oldnum;
	
	// pointer to current value
	int* num;
	
	// pointer to bool_t stating
	//  whether to update number
	bool_t* on;
	
	// list of patches for 0-9
	patch_t** p;
	
	// user data
	int data;
	
} st_number_t;

// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
typedef struct
{
	// number information
	st_number_t n;
	
	// percent sign graphic
	patch_t* p;
	
} st_percent_t;

// Multiple Icon widget
typedef struct
{
	// center-justified location of icons
	int x;
	int y;
	
	// last icon number
	int oldinum;
	
	// pointer to current icon
	int* inum;
	
	// pointer to bool_t stating
	//  whether to update icon
	bool_t* on;
	
	// list of icons
	patch_t** p;
	
	// user data
	int data;
	
} st_multicon_t;

// Binary Icon widget

typedef struct
{
	// center-justified location of icon
	int x;
	int y;
	
	// last icon value
	int oldval;
	
	// pointer to current icon status
	bool_t* val;
	
	// pointer to boolean
	//  stating whether to update icon
	bool_t* on;
	
	patch_t* p;					// icon
	int data;					// user data
	
} st_binicon_t;

extern patch_t* sttminus;		// from ST_lib

//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
void STlib_init(void);

// Number widget routines
void STlib_initNum(st_number_t* n, int x, int y, patch_t** pl, int* num, bool_t* on, int width);

void STlib_updateNum(st_number_t* n, bool_t refresh);

// Percent widget routines
void STlib_initPercent(st_percent_t* p, int x, int y, patch_t** pl, int* num, bool_t* on, patch_t* percent);

void STlib_updatePercent(st_percent_t* per, int refresh);

// Multiple Icon widget routines
void STlib_initMultIcon(st_multicon_t* mi, int x, int y, patch_t** il, int* inum, bool_t* on);

void STlib_updateMultIcon(st_multicon_t* mi, bool_t refresh);

// Binary Icon widget routines

void STlib_initBinIcon(st_binicon_t* b, int x, int y, patch_t* i, bool_t* val, bool_t* on);

void STlib_updateBinIcon(st_binicon_t* bi, bool_t refresh);

#endif
