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
		a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(0x1F);
		a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(0x1F);
		a_Out[3] = (a_Op >> UINT32_C(11)) & UINT32_C(0x1F);
		a_Out[4] = (a_Op >> UINT32_C(6)) & UINT32_C(0x1F);
		a_Out[5] = (a_Op) & UINT32_C(0x3F);
		return;
	}
	
	/* Only 3 instructions use the jump format */
	if ((a_Out[0] & UINT32_C(0x2)) == UINT32_C(0x2) || a_Out[0] == UINT32_C(0x1A))
	{
		// 26 bits of absolute stuff
		a_Out[1] = (a_Op) & UINT32_C(0x3FFFFFF);
		return;
	}
	
	/* Otherwise, immediate mode */
	// 33222222 22221111 111111
	// 10987654 32109876 54321098 76543210
	// ooooooss sssttttt iiiiiiii iiiiiiii
	a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(0x1F);
	a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(0x1F);
	a_Out[3] = (a_Op) & UINT32_C(0xFFFF);
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
	uint32_t Dec[6];
	MIPS_Map_t* Map;
	
	/* Run count opcodes */
	for (i = 0; i < a_Count; i++)
	{
		// Read memory at PC
		if (!(Map = MIPS_VMGetAddr(a_VM, a_VM->CPU.pc & (~UINT32_C(3)), &BaseOff)))
			Op = 0;	// just use NULL opcode
		
		// Obtain opcode
		else
		{
			Op = ((uint32_t*)Map->RealMem)[BaseOff];
#if defined(__REMOOD_BIG_ENDIAN)
			Op = LittleSwapUInt32(op);
#endif
		}
		
		// Decode opcode
		MIPS_DecodeOp(Op, Dec);
		
		// Which opcode?
		switch (Dec[0])
		{
			// Arithmetic
			case 0:
				switch (Dec[5])
				{
						// Unknown
					default:
						break;
				}
				break;
			
				// Illegals
			case 1:
				break;
				
				// Jump
			case 2:
				PRINTOP(("j %x\n", Dec[1]));
				a_VM->CPU.pc += Dec[1] << UINT32_C(2);
				break;
			
				// Unknown
			default:
				break;
		}
		
		// Increase PC by 4
		a_VM->CPU.pc += UINT32_C(4);
		a_VM->CPU.pc &= ~UINT32_C(3);
	}
	
	/* No Exceptions met */
	return true;
}

