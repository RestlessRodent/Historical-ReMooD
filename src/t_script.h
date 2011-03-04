// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright(C) 2000 Simon Howard
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

#ifndef __T_SCRIPT_H__
#define __T_SCRIPT_H__

typedef struct runningscript_s runningscript_t;

#include "p_mobj.h"
#include "t_parse.h"
#include "command.h"
#include "console.h"

struct runningscript_s
{
	script_t *script;

	// where we are
	char *savepoint;

	enum
	{
		wt_none,				// not waiting
		wt_delay,				// wait for a set amount of time
		wt_tagwait,				// wait for sector to stop moving
		wt_scriptwait,			// wait for script to finish
	} wait_type;
	int wait_data;				// data for wait: tagnum, counter, script number etc

	// saved variables
	svariable_t *variables[VARIABLESLOTS];

	runningscript_t *prev, *next;	// for chain
	mobj_t *trigger;
};

void T_Init();
void T_ClearScripts();
void T_RunScript(int n);
void T_RunThingScript(int);
void T_PreprocessScripts();
void T_DelayedScripts();
mobj_t *MobjForSvalue(svalue_t svalue);

		// console commands
void T_Dump();
void T_ConsRun();

extern script_t levelscript;
//extern script_t *scripts[MAXSCRIPTS];       // the scripts
extern mobj_t *t_trigger;

void T_AddCommands();

extern consvar_t cv_scr_allowcommandexec;

#endif
