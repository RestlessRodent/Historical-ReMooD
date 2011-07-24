// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 2011 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: XML Menu Code

#ifndef __M_MENU_H__
#define __M_MENU_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "d_event.h"
#include "w_wad.h"
#include "d_rmod.h"
#include "z_zone.h"

/****************
*** CONSTANTS ***
****************/

/* MessageMode_t -- Type of message to display */
typedef enum MessageMode_e
{
	MM_NOTHING,										// Nothing special
	MM_WARNING,										// Warning message
	MM_ERROR,										// Error Message
	
	NUMMESSAGEMODES
} MessageMode_t;

/*********************
*** GUI PROTOTYPES ***
*********************/

/**********************
*** MENU PROTOTYPES ***
**********************/

boolean M_LoadMenuTable(Z_Table_t* const a_Table, const char* const a_ID, void* const a_Data);

void M_SpawnMenu(const char* const Name, const size_t a_PlayerID);
const char* M_ActiveMenu(const size_t a_PlayerID);
void M_StartMessage(const char* const a_Str, void* A_Unk, const MessageMode_t a_Mode);

boolean M_Responder(event_t* const Event);
void M_Ticker(void);
void M_Drawer(void);

#endif /* __M_MENU_H__ */

