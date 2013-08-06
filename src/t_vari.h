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
// Copyright(C) 2000 Simon Howard
// Copyright (C) 2008-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: FraggleScript files...
// #############################################################################
// ##  THIS SOURCE FILE HAS BEEN DEPRECATED AND WILL BE REMOVED IN THE FUTURE ##
// #############################################################################
// # There should be no futher changes unless necessary. Future dependencies   #
// # on this code will be changed, replaced and/or removed.                    #
// #############################################################################
// # NOTE: Deprecated and will be replaced by the new ReMooD Virtual Machine   #
// #       Which will be the heart of ReMooD Script. Of course there is a      #
// #       Legacy compatibility layer that will be maintained so all those     #
// #       awesome Legacy mods can be played.                                  #
// #############################################################################

#ifndef __VARIABLE_H__
#define __VARIABLE_H__

typedef struct svariable_s svariable_t;

#define VARIABLESLOTS 16

#include "t_parse.h"
#include "p_mobj.h"
#include "m_fixed.h"

// hash the variables for speed: this is the hashkey

#define variable_hash(n)                \
              (   ( (n)[0] + (n)[1] +   \
                   ((n)[1] ? (n)[2] +   \
                   ((n)[2] ? (n)[3]  : 0) : 0) ) % VARIABLESLOTS )

// svariable_t
struct svariable_s
{
	char* name;
	int type;					// vt_string or vt_int: same as in svalue_t
	union
	{
		char* s;
		long i;
		mobj_t* mobj;
		fixed_t fixed;
		
		char** pS;				// pointer to game string
		int* pI;				// pointer to game int
		fixed_t* pFixed;
		mobj_t** pMobj;			// pointer to game obj
		double* pf;
		
		void (*handler) ();		// for functions
		char* labelptr;			// for labels
	} value;
	svariable_t* next;			// for hashing
};

// variable types

enum
{
	svt_string,
	svt_int,
	svt_fixed,
	svt_mobj,					// a map object
	svt_function,				// functions are stored as variables
	svt_label,					// labels for goto calls are variables
	svt_const,					// const
	svt_pInt,					// pointer to game int
	svt_pFixed,
	svt_pString,				// pointer to game string
	svt_pMobj,					// pointer to game mobj
};

// variables

void T_ClearHubScript();

void init_variables();
svariable_t* new_variable(script_t* script, char* name, int vtype);
svariable_t* find_variable(char* name);
svariable_t* variableforname(script_t* script, char* name);
svalue_t getvariablevalue(svariable_t* v);
void setvariablevalue(svariable_t* v, svalue_t newvalue);
void clear_variables(script_t* script);

svariable_t* add_game_int(char* name, int* var);
svariable_t* add_game_string(char* name, char** var);
svariable_t* add_game_mobj(char* name, mobj_t** mo);

// functions

svalue_t evaluate_function(int start, int stop);	// actually run a function
svariable_t* new_function(char* name, void (*handler) ());

// arguments to handler functions

#define MAXARGS 128
extern int t_argc;
extern svalue_t* t_argv;
extern svalue_t t_return;

#endif
