// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: FraggleScript files...

#ifndef __T_SCRIPT_H__
#define __T_SCRIPT_H__

typedef struct runningscript_s runningscript_t;

#include "p_mobj.h"
#include "t_parse.h"

#include "console.h"

struct runningscript_s
{
	script_t* script;
	
	// where we are
	char* savepoint;
	
	enum
	{
		wt_none,				// not waiting
		wt_delay,				// wait for a set amount of time
		wt_tagwait,				// wait for sector to stop moving
		wt_scriptwait,			// wait for script to finish
	} wait_type;
	int wait_data;				// data for wait: tagnum, counter, script number etc
	
	// saved variables
	svariable_t* variables[VARIABLESLOTS];
	
	runningscript_t* prev, *next;	// for chain
	mobj_t* trigger;
};

void T_Init();
void T_ClearScripts();
void T_RunScript(int n);
void T_RunThingScript(int);
void T_PreprocessScripts();
void T_DelayedScripts();
mobj_t* MobjForSvalue(svalue_t svalue);

// console commands
void T_Dump();
void T_ConsRun();

extern script_t levelscript;

//extern script_t *scripts[MAXSCRIPTS];       // the scripts
extern mobj_t* t_trigger;

void T_AddCommands();

#endif

