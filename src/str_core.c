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
// Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
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
// DESCRIPTION: Stream Reader/Writer

/***************
*** INCLUDES ***
***************/

#include "str.h"

/****************
*** FUNCTIONS ***
****************/

STR_t* STR_Open(const STR_Type_t a_Type, const STR_Class_t a_Class);
bool_t STR_Bind(STR_t* const a_Str, STR_Addr_t* const a_Addr);
bool_t STR_Listen(STR_t* const a_Str, const uint32_t a_ConnCap);
STR_t* STR_Accept(STR_t* const a_Str, STR_Addr_t* const a_Addr);
bool_t STR_Connect(STR_t* const a_Str, STR_Addr_t* const a_Addr);
void STR_Close(STR_t* const a_Str);
STR_t* STR_OpenFile(const char* const a_PathName, const char* const a_Mode);
STR_t* STR_OpenEntry(WL_WADEntry_t* const a_Entry);
STR_Type_t STR_GetType(STR_t* const a_Str);
STR_Class_t STR_GetClass(STR_t* const a_Str);
bool_t STR_GetAddrInfo(const char* const a_Name, STR_AddrInfo_t** const a_Out);
void STR_FreeAddrInfo(STR_AddrInfo_t* const a_Addresses);
bool_t STR_GetNameInfo(STR_AddrInfo_t* const a_In, char* const a_Name, const size_t a_Len);
bool_t STR_GetAddr(const char* const a_Name, STR_Addr_t* const a_Addr);
uint32_t STR_Tell(STR_t* const a_Str);
uint32_t STR_Seek(STR_t* const a_Str, const uint32_t a_Where, const bool_t a_End);
uint32_t STR_Wait(STR_t* const a_Str);
uint32_t STR_ReadFrom(STR_t* const a_Str, uint8_t* const a_In, const uint32_t a_Len, STR_Addr_t* const a_SrcAddr);
uint32_t STR_WriteTo(STR_t* const a_Str, const uint8_t* const a_Out, const uint32_t a_Len, STR_Addr_t* const a_DestAddr);

