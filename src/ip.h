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
// DESCRIPTION: Communication Layer
// 
// This implements a proprietary sub protocol which is then wrapped upon an API
// so that the sub protocol can be changed without breaking all the code!

#ifndef __IP_H__
#define __IP_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "doomdef.h"
#include "i_util.h"

/****************
*** CONSTANTS ***
****************/

/* IP_Flags_t -- IP Flags */
typedef enum IP_Flags_e
{
	IPF_INPUT 			= UINT32_C(0x00000001),	// Can be connected to
} IP_Flags_t;

/*****************
*** STRUCTURES ***
*****************/

struct IP_Proto_s;
struct IP_Conn_s;
struct IP_Addr_s;

typedef bool_t (*IP_VerifyF_t)(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags);
typedef struct IP_Conn_s* (*IP_CreateF_t)(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags);

/* IP_Proto_t -- Protocol Handler */
typedef struct IP_Proto_s
{
	const char* Name;							// Name of protocol
	
	IP_VerifyF_t VerifyF;						// Verify Flags
	IP_CreateF_t CreateF;						// Create Connection
} IP_Proto_t;

/* IP_Addr_t -- Standard Address */
typedef struct IP_Addr_s
{
	const struct IP_Proto_s* Handler;			// Handler Used
} IP_Addr_t;

/* IP_Conn_t -- Protocol Connection */
typedef struct IP_Conn_s
{
	const struct IP_Proto_s* Handler;			// Handler Used
	uint32_t Flags;								// Connection Flags
	uint32_t UUID;								// Connection ID
} IP_Conn_t;

/*****************
*** PROTOTYPES ***
*****************/

void IP_Init(void);

const struct IP_Proto_s* IP_ProtoByName(const char* const a_Name);

struct IP_Conn_s* IP_Create(const char* const a_URI, const uint32_t a_Flags);
void IP_Destroy(struct IP_Conn_s* const a_Conn);

struct IP_Conn_s* IP_ConnById(const uint32_t a_UUID);

void IP_ConnRun(struct IP_Conn_s* const a_Conn);

#endif /* __IP_H__ */


