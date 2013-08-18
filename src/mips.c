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

/***************
*** INCLUDES ***
***************/

#include "mips.h"
#include "z_zone.h"
#include "console.h"

/*****************
*** STRUCTURES ***
*****************/

#if defined(_DEBUG)
	#define PRINTOP(x) (a_PrintOp ? CONL_PrintF x : (void)1337 )
#else
	#define PRINTOP(x)
#endif

/*******************
*** OPCODE TABLE ***
*******************/

/****************
*** FUNCTIONS ***
****************/

/* MIPS_VMAddMap() -- Adds virtual memory mapping */
bool_t MIPS_VMAddMap(MIPS_VM_t* const a_VM, void* const a_Real, const uint_fast32_t a_Fake, const uint_fast32_t a_Len, const uint_fast32_t a_Flags)
{
	MIPS_Map_t* New;
	
	/* Check */
	if (!a_VM || !a_Real || !a_Len)
		return false;
	
	/* Add room for 1 more map */
	Z_ResizeArray((void**)&a_VM->Maps, sizeof(*a_VM->Maps), a_VM->NumMaps, a_VM->NumMaps + 1);
	New = &a_VM->Maps[a_VM->NumMaps++];
	
	/* Place data here */
	// Align to 4 bytes
	New->Len = a_Len & (~UINT32_C(3));
	New->VMOff = a_Fake & (~UINT32_C(3));
	New->RealMem = a_Real;
	New->Flags = a_Flags;
	
	/* Success */
	return true;
}

/* MIPS_VMGetAddr() -- Obtain memory address from VM location */
static inline MIPS_Map_t* MIPS_VMGetAddr(MIPS_VM_t* const a_VM, const uint_fast32_t a_Addr, uint32_t* const a_BaseOfp)
{
	register int_fast32_t i;
	register uint_fast32_t BaseAddr;
	
	/* Look in memory maps */
	for (i = 0; i < a_VM->NumMaps; i++)
		if (a_Addr >= a_VM->Maps[i].VMOff)
		{
			BaseAddr = a_Addr - a_VM->Maps[i].VMOff;
			
			if (BaseAddr < a_VM->Maps[i].Len)
			{
				if (a_BaseOfp)
					*a_BaseOfp = BaseAddr & (~UINT32_C(3));
				return &a_VM->Maps[i];
			}
		}
	
	/* Not found */
	return NULL;
}

/* MIPS_DecodeOp() -- Decodes opcode */
static inline void MIPS_DecodeOp(const uint32_t a_Op, uint32_t* const a_Out)
{
	/* Decode function */
	a_Out[0] = (a_Op >> UINT32_C(26)) & UINT32_C(0x3F);
	
	/* If function is zero, always arithmetic */
	if (!a_Out[0])
	{
		// 33222222 22221111 111111
		// 10987654 32109876 54321098 76543210
		// ooooooss sssttttt dddddaaa aaffffff
		a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(0x1F);	// s
		a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(0x1F);	// t
		a_Out[3] = (a_Op >> UINT32_C(11)) & UINT32_C(0x1F);	// d
		a_Out[4] = (a_Op >> UINT32_C(6)) & UINT32_C(0x1F);	// a
		a_Out[5] = (a_Op) & UINT32_C(0x3F);					// f
		return;
	}
	
	/* Only 3 instructions use the jump format */
	if ((a_Out[0] & UINT32_C(0x2)) == UINT32_C(0x2) || a_Out[0] == UINT32_C(0x1A))
	{
		// 26 bits of absolute stuff
		a_Out[3] = (a_Op) & UINT32_C(0x3FFFFFF);			// i
		return;
	}
	
	/* Otherwise, immediate mode */
	// 33222222 22221111 111111
	// 10987654 32109876 54321098 76543210
	// ooooooss sssttttt iiiiiiii iiiiiiii
	a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(0x1F);		// s
	a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(0x1F);		// t
	a_Out[3] = (a_Op) & UINT32_C(0xFFFF);					// i
}

/* MIPS_VMRun() -- Runs virtual machine, for count opcodes */
bool_t MIPS_VMRunX(MIPS_VM_t* const a_VM, const uint_fast32_t a_Count
#if defined(_DEBUG)
	, const bool_t a_PrintOp
#endif
	)
{
	register uint_fast32_t i;
	uint32_t Op, BaseOff;
	uint32_t Am[6];
	MIPS_Map_t* Map;

#if defined(_DEBUG)
	MIPS_CPU_t OldCPU;
	uint32_t x;
#endif
	
	/* Run count opcodes */
	for (i = 0; i < a_Count; i++)
	{
		// Read memory at PC
		if (!(Map = MIPS_VMGetAddr(a_VM, a_VM->CPU.pc & (~UINT32_C(3)), &BaseOff)))
			Op = UINT32_C(0xFFFFFFFF);
		
		// Obtain opcode
		else
		{
			Op = ((uint32_t*)Map->RealMem)[BaseOff];
#if defined(__REMOOD_BIG_ENDIAN)
			Op = LittleSwapUInt32(op);
#endif
		}
		
		// Decode opcode
		MIPS_DecodeOp(Op, Am);
		
		// Always reset register zero to zero
		a_VM->CPU.r[0] = 0;
		
#if defined(_DEBUG)
		OldCPU = a_VM->CPU;
#endif
		
		// Which opcode?
		switch (Am[0])
		{
/*---------------------------------------------------------------------------*/
// ENTERING UGLY ZONE, MACROS USED TO MAKE IT LOOK NICER
#define A(n) Am[(n)]
#define R(n) a_VM->CPU.r[(n)]
#define AR(n) a_VM->CPU.r[Am[(n)]]
#define PC a_VM->CPU.pc
#define BEGINARITH case 0: switch (Am[5]) {
#define ENDARITH } break;

case 2:		PRINTOP(("j %08x\n", A(3)));
	PC += A(3) << UINT32_C(2);
	break;

case 3:		PRINTOP(("jal %08x\n", A(3)));
	R(31) = PC;
	PC += A(3) << UINT32_C(2);
	break;

case 4:		PRINTOP(("beq $%u, $%u, %08x\n", A(1), A(2), A(3)));
	if (AR(1) == AR(2))
		PC += A(3) << UINT32_C(2);
	break;

case 5:		PRINTOP(("bne $%u, $%u, $08x\n", A(1), A(2), A(3)));
	if (AR(1) != AR(2))
		PC += A(3) << UINT32_C(2);
	break;

BEGINARITH

case 0:		PRINTOP(("sll $%u, $%u, %u\n", A(3), A(2), A(4)));
	AR(3) = AR(2) << A(4);
	break;

case 2:		PRINTOP(("srl $%u, $%u, %u\n", A(3), A(2), A(4)));
	AR(3) = AR(2) >> A(4);
	break;

case 3:		PRINTOP(("sra $%u, $%u, %u\n", A(3), A(2), A(4)));
	AR(3) = ((int32_t)AR(2)) >> ((int32_t)A(4));
	break;

case 4:		PRINTOP(("sllv $%u, $%u, $%u\n", A(3), A(2), A(1)));
	AR(3) = AR(2) << AR(1);
	break;

case 6:		PRINTOP(("srlv $%u, $%u, $%u\n", A(3), A(2), A(1)));
	AR(3) = AR(2) >> AR(1);
	break;
	
case 7:		PRINTOP(("srav $%u, $%u, $%u\n", A(3), A(2), A(1)));
	AR(3) = ((int32_t)AR(2)) >> ((int32_t)AR(1));
	break;

case 8:		PRINTOP(("jr $%u\n", A(1)));
	PC = AR(1);
	break;

case 9:		PRINTOP(("jalr $%u\n", A(1)));
	R(31) = PC;
	PC = AR(1);

default:
	//CONL_PrintF("Unknown Arith %i (%08x)\n", Am[5], Op);
	break;

ENDARITH

default:
	//CONL_PrintF("Unknown opcode %i (%08x)\n", Am[0], Op);
	break;

#undef PC
#undef R
/*---------------------------------------------------------------------------*/
		}
		
		// Always reset register zero to zero
		a_VM->CPU.r[0] = 0;
		
		// Print changes in CPU registers
#if defined(_DEBUG)
		for (x = 0; x < 32; x++)
			if (OldCPU.r[x] != a_VM->CPU.r[x])
				CONL_PrintF("r%i = (%08x -> %08x)\n", x, OldCPU.r[x], a_VM->CPU.r[x]);
#endif
		
		// Increase PC by 4
		a_VM->CPU.pc += UINT32_C(4);
		a_VM->CPU.pc &= ~UINT32_C(3);
	}
	
	/* No Exceptions met */
	return true;
}

