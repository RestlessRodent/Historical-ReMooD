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
// DESCRIPTION: MIPS Emulator/Simulator

#ifndef __MIPS_H__
#define __MIPS_H__

/*****************************************************************************/

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"

/****************
*** CONSTANTS ***
****************/

/* MIPS_Regs_t -- MIPS Registers */
typedef enum MIPS_Regs_e
{
	/* Numeric Registers */
	MIPS_R0,
	MIPS_R1,
	MIPS_R2,
	MIPS_R3,
	MIPS_R4,
	MIPS_R5,
	MIPS_R6,
	MIPS_R7,
	MIPS_R8,
	MIPS_R9,
	MIPS_R10,
	MIPS_R11,
	MIPS_R12,
	MIPS_R13,
	MIPS_R14,
	MIPS_R15,
	MIPS_R16,
	MIPS_R17,
	MIPS_R18,
	MIPS_R19,
	MIPS_R20,
	MIPS_R21,
	MIPS_R22,
	MIPS_R23,
	MIPS_R24,
	MIPS_R25,
	MIPS_R26,
	MIPS_R27,
	MIPS_R28,
	MIPS_R29,
	MIPS_R30,
	MIPS_R31,
	
	/* Aliased */
	MIPS_ZERO = MIPS_R0,
	MIPS_AT,
	MIPS_V0,
	MIPS_V1,
	MIPS_A0,
	MIPS_A1,
	MIPS_A2,
	MIPS_A3,
	MIPS_T0,
	MIPS_T1,
	MIPS_T2,
	MIPS_T3,
	MIPS_T4,
	MIPS_T5,
	MIPS_T6,
	MIPS_T7,
	MIPS_S0,
	MIPS_S1,
	MIPS_S2,
	MIPS_S3,
	MIPS_S4,
	MIPS_S5,
	MIPS_S6,
	MIPS_S7,
	MIPS_T8,
	MIPS_T9,
	MIPS_K0,
	MIPS_K1,
	MIPS_GP,
	MIPS_SP,
	MIPS_FP,
	MIPS_RA,
	
	/* Other aliases */
	MIPS_S8 = MIPS_FP,
} MIPS_Regs_t;

/*****************
*** STRUCTURES ***
*****************/

/* MIPS_CPU_t -- MIPS CPU Status */
typedef struct MIPS_CPU_s
{
	uint32_t Reg[32];							// MIPS Registers
	uint32_t pc;								// PC register
} MIPS_CPU_t;

/* MIPS_Map_t -- MIPS Memory Mapping */
typedef struct MIPS_Map_s
{
	uint32_t Len;								// Length of mapping
	uint32_t VMOff;								// VM Offset
	void* RealMem;								// Real Memory
} MIPS_Map_t;

/* MIPS_VM_t -- MIPS Virtual Machine */
typedef struct MIPS_VM_s
{
	MIPS_CPU_t CPU;								// CPU Status
	
	// Real buffer memory maps
	MIPS_Map_t* Maps;							// Memory maps
	int32_t NumMaps;							// Number of memory maps
} MIPS_VM_t;

/****************
*** FUNCTIONS ***
****************/

bool_t MIPS_VMAddMap(MIPS_VM_t* const a_VM, void* const a_Real, const uint32_t a_Fake, const uint32_t a_Len);
void MIPS_VMRun(MIPS_VM_t* const a_VM, const uint32_t a_Count);

/*****************************************************************************/

#endif /* __MIPS_H__ */


