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
// Copyright (C) 2012-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Private IP Stuff

#ifndef __IP_PRV_H__
#define __IP_PRV_H__

/***************
*** INCLUDES ***
***************/

#include "ip.h"

/*****************
*** PROTOTYPES ***
*****************/

/* ReMooD Protocol */
bool_t IP_RMD_VerifyF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags);
struct IP_Conn_s* IP_RMD_CreateF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags);
void IP_RMD_RunConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn);
void IP_RMD_DeleteConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn);

/* Odamex Protocol */
bool_t IP_ODA_VerifyF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags);
struct IP_Conn_s* IP_ODA_CreateF(const struct IP_Proto_s* a_Proto, const char* const a_Host, const uint32_t a_Port, const char* const a_Options, const uint32_t a_Flags);
void IP_ODA_RunConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn);
void IP_ODA_DeleteConnF(const struct IP_Proto_s* a_Proto, struct IP_Conn_s* const a_Conn);

/* UDP Baseline */
bool_t IP_UDPResolveHost(const struct IP_Proto_s* a_Proto, IP_Addr_t* const a_Dest, const char* const a_Name, const uint32_t a_Port);

#endif /* __IP_PRV_H__ */


