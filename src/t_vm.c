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
// Copyright (C) 2012 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Virtual Machine
/***************
*** INCLUDES ***
***************/

#include "t_vm.h"
#include "dstrings.h"
#include "console.h"

/****************
*** FUNCTIONS ***
****************/

/* TVM_Clear() -- Clears the VM */
void TVM_Clear(void)
{
}

/* TVM_CompileWLES() -- Compiles stream segment */
void TVM_CompileWLES(WL_ES_t* const a_Stream, const size_t a_End)
{
	/* Check */
	if (!a_Stream || !a_End)
		return;
	
	/* Message */
	CONL_OutputUT(CT_SCRIPTING, DSTR_TVMC_COMPILING, "%s\n", WL_StreamGetEntry(a_Stream)->Name);
}

/* TVM_CompileEntry() -- Compiles entry */
void TVM_CompileEntry(const WL_WADEntry_t* const a_Entry)
{
	WL_ES_t* ScriptStream;
	
	/* Open stream */
	ScriptStream = WL_StreamOpen(a_Entry);
	
	/* If it was created */
	if (ScriptStream)
	{
		// Check Unicode viability
		WL_StreamCheckUnicode(ScriptStream);
		
		// Compile it
		TVM_CompileWLES(ScriptStream, a_Entry->Size);
		
		// Close it
		WL_StreamClose(ScriptStream);
	}
}

