// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Savegame I/O, archiving, persistence.

#ifndef __P_SAVEG__
#define __P_SAVEG__

/***************
*** INCLUDES ***
***************/

#include "d_block.h"

/****************
*** CONSTANTS ***
****************/

/* P_SaveSubVersion_t -- Save game sub version format */
typedef enum P_SaveSubVersion_s
{
	// Add more as time goes on...	
	
	PSSV_LATEST
} P_SaveSubVersion_t;

/****************
*** FUNCTIONS ***
****************/

void P_InitSGConsole(void);

// Persistent storage/archiving.
// These are the load / save game routines.

void P_SaveGame(void);
bool_t P_LoadGame(void);

bool_t P_LoadGameEx(const char* FileName, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin);
bool_t P_SaveGameEx(const char* Desc, const char* ExtFileName, size_t ExtFileNameLen, size_t* SaveLen, uint8_t** Origin);

bool_t P_LoadGameFromBS(D_BS_t* const a_Stream, I_HostAddress_t* const a_NetAddr);
bool_t P_SaveGameToBS(D_BS_t* const a_Stream, I_HostAddress_t* const a_NetAddr);

bool_t P_SGDXSpec(D_BS_t* const a_Stream, I_HostAddress_t* const a_NetAddr, bool_t a_Load);

extern uint8_t* save_p;

bool_t P_SaveToStream(D_BS_t* const a_Str, D_BS_t* const a_OrigStr);
bool_t P_LoadFromStream(D_BS_t* const a_Str, const bool_t a_DemoPlay);

#endif

