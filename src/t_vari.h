// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: FraggleScript files...

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
