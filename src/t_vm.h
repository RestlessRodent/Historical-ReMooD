// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Virtual Machine

#ifndef __T_VM_H__
#define __T_VM_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "w_wad.h"

/****************
*** CONSTANTS ***
****************/

/* TVM_Namespace_t -- VM namespace */
typedef enum TVM_Namespace_e
{
	TVMNS_GLOBAL,								// Global
	TVMNS_LEVEL,								// Level
	
	NUMTVMNAMESPACES
} TVM_Namespace_t;

/*****************
*** STRUCTURES ***
*****************/

/* TVM_CCVarInfo_t -- Compiler variable info */
typedef struct TVM_CCVarInfo_e
{
	char* const Name;							// Base name of variable
	char* const Type;							// Type of variable
} TVM_CCVarInfo_t;

/****************
*** FUNCTIONS ***
****************/

void TVM_Clear(const TVM_Namespace_t a_NameSpace);
void TVM_CompileWLES(const TVM_Namespace_t a_NameSpace, WL_ES_t* const a_Stream, const uint32_t a_End);
void TVM_CompileEntry(const TVM_Namespace_t a_NameSpace, const WL_WADEntry_t* const a_Entry);

#endif /* __T_VM_H__ */

