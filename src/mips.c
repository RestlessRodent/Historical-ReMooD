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

/****************
*** CONSTANTS ***
****************/

#if defined(_DEBUG)
static const char* l_RegNames[32] =
{
	"zero",
	"at",
	"v0",
	"v1",
	"a0",
	"a1",
	"a2",
	"a3",
	"t0",
	"t1",
	"t2",
	"t3",
	"t4",
	"t5",
	"t6",
	"t7",
	"s0",
	"s1",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7",
	"t8",
	"t9",
	"k0",
	"k1",
	"gp",
	"sp",
	"s8",
	"ra",
};
#endif

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

/* MIPS_VMAddMapFunc() -- Add mapping function */
bool_t MIPS_VMAddMapFunc(MIPS_VM_t* const a_VM, MIPS_VMMapReadFunc_t a_ReadFunc, MIPS_VMMapWriteFunc_t a_WriteFunc, const uint_fast32_t a_Fake, const uint_fast32_t a_Len, const uint_fast32_t a_Flags)
{
	MIPS_Map_t* New;
	
	/* Check */
	if (!a_VM || !a_ReadFunc || !a_Len)
		return false;
	
	/* Add room for 1 more map */
	Z_ResizeArray((void**)&a_VM->Maps, sizeof(*a_VM->Maps), a_VM->NumMaps, a_VM->NumMaps + 1);
	New = &a_VM->Maps[a_VM->NumMaps++];
	
	/* Place data here */
	// Align to 4 bytes
	New->Len = a_Len & (~UINT32_C(3));
	New->VMOff = a_Fake & (~UINT32_C(3));
	New->ReadFunc = a_ReadFunc;
	New->WriteFunc = a_WriteFunc;
	New->Flags = a_Flags;
	
	/* Success */
	return true;
}

/* MIPS_VMGetMap() -- Obtain map for address */
static inline MIPS_Map_t* MIPS_VMGetMap(MIPS_VM_t* const a_VM, const uint_fast32_t a_Addr, uint32_t* const a_BaseOfp)
{
	register int_fast32_t i;
	register uint_fast32_t BaseAddr;
	
	/* Look in memory maps */
	for (i = 0; i < a_VM->NumMaps; i++)
		if (a_Addr >= a_VM->Maps[i].VMOff)
		{
			BaseAddr = a_Addr - a_VM->Maps[i].VMOff;
			
			if (BaseAddr <= a_VM->Maps[i].Len)
			{
				*a_BaseOfp = BaseAddr;
				return &a_VM->Maps[i];
			}
		}
	
	/* Not found */
	return NULL;
}

/* MIPS_ReadMem() -- Reads from memory */
static inline uint32_t MIPS_ReadMem(MIPS_VM_t* const a_VM, const uint_fast32_t a_Addr, const uint_fast32_t a_Width)
{
	MIPS_Map_t* Map;
	uint32_t BaseOff;
	register uint32_t RetVal;
	
	/* Find map first */
	BaseOff = 0;
	if (!(Map = MIPS_VMGetMap(a_VM, a_Addr, &BaseOff)))
		return 0;	// Failed to find
	
	/* Not Readable */
	if (!(Map->Flags & MIPS_MFR))
		return 0;
	
	/* Memory Mapped */
	if (Map->RealMem)
	{
		if (a_Width == 4)
		{
			RetVal = LittleSwapUInt32(((uint32_t*)Map->RealMem)[BaseOff >> 2]);
			return RetVal;
		}
		else if (a_Width == 2)
			return LittleSwapUInt16(((uint16_t*)Map->RealMem)[BaseOff >> 1]);
		else
			return ((uint8_t*)Map->RealMem)[BaseOff];
	}
	
	/* Function Mapped */
	else
	{
		RetVal = Map->ReadFunc(a_VM, Map, BaseOff & (~UINT32_C(3)));
		
		if (a_Width == 4)
			return RetVal;
		else if (a_Width == 2)
		{
			// read of higher half word
			if ((BaseOff & UINT32_C(3)) != UINT32_C(0))
				RetVal >>= UINT32_C(16);
			
			return RetVal & UINT32_C(0x0000FFFF);
		}
		else
		{
			// Read of higher quarter words
				// 0 = 0
				// 1 = 8
				// 2 = 16
				// 3 = 24
			RetVal >>= UINT32_C(8) * (UINT32_C(8) * (BaseOff & UINT32_C(3)));
			return RetVal & UINT32_C(0x000000FF);
		}
	}
}

/* MIPS_ReadMemX() -- Reads from memory (external) */
uint32_t MIPS_ReadMemX(MIPS_VM_t* const a_VM, const uint_fast32_t a_Addr, const uint_fast32_t a_Width)
{
	/* Check */
	if (!a_VM)
		return 0;
	
	return MIPS_ReadMem(a_VM, a_Addr, a_Width);
}

/* MIPS_WriteMem() -- Writes to memory */
static inline void MIPS_WriteMem(MIPS_VM_t* const a_VM, const uint_fast32_t a_Addr, const uint_fast32_t a_Width, const uint32_t a_Val)
{
	MIPS_Map_t* Map;
	uint32_t BaseOff;
	union
	{
		uint8_t u8[4];
		uint16_t u16[2];
		uint32_t u32;
	} MemVal;
	
	/* Find map first */
	BaseOff = 0;
	if (!(Map = MIPS_VMGetMap(a_VM, a_Addr, &BaseOff)))
		return;	// Failed to find
	
	/* Not Writeable */
	if (!(Map->Flags & MIPS_MFW))
		return;
	
	/* Memory Mapped */
	if (Map->RealMem)
	{
		if (a_Width == 4)
			((uint32_t*)Map->RealMem)[BaseOff >> 2] = LittleSwapUInt32(a_Val);
		else if (a_Width == 2)
			((uint16_t*)Map->RealMem)[BaseOff >> 1] = LittleSwapUInt16(((uint16_t)a_Val));
		else
			((uint8_t*)Map->RealMem)[BaseOff] = a_Val;
	}
	
	/* Function Mapped */
	else if (Map->WriteFunc)
	{
		// 32-bit is 1:1 operation
		if (a_Width == 4)
			MemVal.u32 = a_Val;
		
		// 8-bit/16-bit requires a bit more work
		else
		{
			// Read value at memory
			MemVal.u32 = Map->ReadFunc(a_VM, Map, BaseOff & (~UINT32_C(3)));
			
			// 16-bit
			if (a_Width == 2)
			{
				// Higher Half
				if ((BaseOff & UINT32_C(3)) != UINT32_C(0))
#if defined(__REMOOD_BIG_ENDIAN)
					MemVal.u16[1] = a_Val;
#else
					MemVal.u16[0] = a_Val;
#endif
				
				// Lower Half
				else
#if defined(__REMOOD_BIG_ENDIAN)
					MemVal.u16[0] = a_Val;
#else
					MemVal.u16[1] = a_Val;
#endif
			}
			
			// 8-bit
			else
				MemVal.u8[
#if defined(__REMOOD_BIG_ENDIAN)
					(3 - (BaseOff & 3))	// higher addr == lower bit
#else
					(BaseOff & 3)		// higher addr == higher bit
#endif
					] = a_Val;
		}
		
		// Write 32-bit value
		Map->WriteFunc(a_VM, Map, BaseOff & (~UINT32_C(3)), MemVal.u32);
	}
}

/* MIPS_WriteMemX() -- Externalized WriteMem() */
void MIPS_WriteMemX(MIPS_VM_t* const a_VM, const uint_fast32_t a_Addr, const uint_fast32_t a_Width, const uint32_t a_Val)
{
	/* Check */
	if (!a_VM)
		return;
	
	return MIPS_WriteMem(a_VM, a_Addr, a_Width, a_Val);
}

/* MIPS_DecodeOp() -- Decodes opcode */
static inline void MIPS_DecodeOp(const uint32_t a_Op, uint32_t* const a_Out)
{
	/* Decode function */
	a_Out[0] = (a_Op >> UINT32_C(26)) & UINT32_C(63);
	
	/* If function is zero, always arithmetic */
	if (!a_Out[0])
	{
		// 33222222 22221111 111111
		// 10987654 32109876 54321098 76543210
		// ooooooss sssttttt dddddaaa aaffffff
		a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(31);	// s
		a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(31);	// t
		a_Out[3] = (a_Op >> UINT32_C(11)) & UINT32_C(31);	// d
		a_Out[4] = (a_Op >> UINT32_C(6)) & UINT32_C(31);	// a
		a_Out[5] = (a_Op) & UINT32_C(63);					// f
		return;
	}
	
	/* Only 3 instructions use the jump format */
	if (a_Out[0] == UINT32_C(2) || a_Out[0] == UINT32_C(3) || a_Out[0] == UINT32_C(0x1A))
	{
#if defined(_DEBUG)
		a_Out[1] = UINT32_C(0xDEADBEEF);
		a_Out[2] = UINT32_C(0xCAFEBABE);
#endif
		
		// 26 bits of absolute stuff
		a_Out[3] = (a_Op) & UINT32_C(0x3FFFFFF);			// i
		
#if defined(_DEBUG)
		a_Out[4] = UINT32_C(0xFEEDF00D);
		a_Out[5] = UINT32_C(0x1337DEAD);
#endif
		return;
	}
	
	/* Otherwise, immediate mode */
	// 33222222 22221111 111111
	// 10987654 32109876 54321098 76543210
	// ooooooss sssttttt iiiiiiii iiiiiiii
	a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(31);		// s
	a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(31);		// t
	a_Out[3] = (a_Op) & UINT32_C(0xFFFF);					// i
	
#if defined(_DEBUG)
	a_Out[4] = UINT32_C(0xF00D1337);
	a_Out[5] = UINT32_C(0x12345678);
#endif
}

/* MIPS_VMRun() -- Runs virtual machine, for count opcodes */
bool_t MIPS_VMRunX(MIPS_VM_t* const a_VM, const uint_fast32_t a_Count
#if defined(_DEBUG)
	, const bool_t a_PrintOp
#endif
	)
{
	register uint_fast32_t i, j;
	uint32_t Op, BaseOff;
	uint32_t Am[6], NopCount;
	MIPS_Map_t* Map;
	union
	{
		int64_t i;
		uint64_t u;
		
		int32_t i32;
		uint32_t u32;
	} BN;

#if defined(_DEBUG)
	MIPS_CPU_t OldCPU;
	uint32_t x;
#endif
	
	/* Run count opcodes */
	for (NopCount = i = 0; i < a_Count; i++)
	{
		// Handle Jump Delay Slot
		if (a_VM->CPU.jdsactive >= 1)
		{
			// 1 = It was just set
			if (a_VM->CPU.jdsactive == 1)
				a_VM->CPU.jdsactive = 2;
			
			// 2 = It has been set
			else
			{
				// PC is now at ds
				a_VM->CPU.pc = a_VM->CPU.jds;
				
				// Reset to zero
				a_VM->CPU.jds = 0;
				a_VM->CPU.jdsactive = 0;
			}
		}
		
		// Read memory at PC
		Op = MIPS_ReadMem(a_VM, a_VM->CPU.pc, 4);
		
		// Swap operator on big endian
#if defined(__REMOOD_BIG_ENDIAN)
		Op = LittleSwapUInt32(Op);
#endif
		
		// Decode opcode
		MIPS_DecodeOp(Op, Am);
		
		// Always reset register zero to zero
		a_VM->CPU.r[0] = 0;
		
#if defined(_DEBUG)
		OldCPU = a_VM->CPU;
		
		if (a_PrintOp)
			CONL_PrintF("%08x (@%08x): ", Op, a_VM->CPU.pc);
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
#define HI a_VM->CPU.hi
#define LO a_VM->CPU.lo
#define BEGINARITH case 0: switch (Am[5]) {
#define ENDARITH } break;
#define FOUR UINT32_C(4)
#define ADVPC PC += FOUR
#define JDS a_VM->CPU.jds
#define JDSA a_VM->CPU.jdsactive

case 1:	// TODO FIXME: CONFIRM
	if (A(2) == UINT32_C(0x10))
		PRINTOP(("bltzal %s, %4x", l_RegNames[A(1)], A(3)));
	else
		PRINTOP(("bltz %s, %4x", l_RegNames[A(1)], A(3)));
	
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (A(2) == UINT32_C(0x10))
		R(31) = PC + UINT32_C(8);
		
	if (AR(1) == UINT32_C(0) || (AR(1) & UINT32_C(0x80000000)))
	{
		JDS = PC + (BN.u32 << UINT32_C(2));
		JDSA = 1;
	}

	ADVPC;
	break;

case 2:		PRINTOP(("j 0x%08x\n", A(3) << UINT32_C(2)));
	JDS = (PC & UINT32_C(0xF0000000)) | (A(3) << UINT32_C(2));
	JDSA = 1;
	ADVPC;
	break;

case 3:		PRINTOP(("jal 0x%08x\n", A(3) << UINT32_C(2)));
	R(31) = PC + UINT32_C(8);
	JDS = (PC & UINT32_C(0xF0000000)) | (A(3) << UINT32_C(2));
	JDSA = 1;
	ADVPC;
	break;

case 4:		PRINTOP(("beq %s, %s, %hi\n", l_RegNames[A(1)], l_RegNames[A(2)], A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (AR(1) == AR(2))
	{
		JDS = PC + (BN.u32 << UINT32_C(2));
		JDSA = 1;
	}
	
	ADVPC;
	break;

case 5:		PRINTOP(("bne %s, %s, %hi\n", l_RegNames[A(1)], l_RegNames[A(2)], A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (AR(1) != AR(2))
	{
		JDS = PC + (BN.u32 << UINT32_C(2));
		JDSA = 1;
	}
	
	ADVPC;
	break;

case 6:		PRINTOP(("blez %s, %hi\n", l_RegNames[A(1)], A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if ((AR(1) & UINT32_C(0x80000000)) || AR(1) == 0)
	{
		JDS = PC + (BN.u32 << UINT32_C(2));
		JDSA = 1;
	}
	
	ADVPC;
	break;

case 7:	PRINTOP(("bgtz %s, %i\n", l_RegNames[A(1)], A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (((AR(1) & UINT32_C(0x80000000)) == 0) && AR(1) != 0)
	{
		JDS = PC + (BN.u32 << UINT32_C(2));
		JDSA = 1;
	}
	
	ADVPC;
	break;

	// addi == addiu, since there are no exceptions
case 8:
case 9:		PRINTOP(("addi%s %s, %s, %i\n", (Am[0] == '8' ? "" : "u"), l_RegNames[A(2)], l_RegNames[A(1)], A(3)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	AR(2) = AR(1) + BN.u32;
	ADVPC;
	break;

case 10:	PRINTOP(("slti %s, %s, %hi\n", l_RegNames[A(2)], l_RegNames[A(1)], A(3)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	// Argument 1 is negative
	if (AR(1) & UINT32_C(0x80000000))
	{
		// If argument two is negative, if 1 is greater
		if (BN.u32 & UINT32_C(0x80000000))
		{
			if (AR(1) > BN.u32)
				AR(2) = UINT32_C(1);
			else
				AR(2) = UINT32_C(0);
		}
		
		// Otherwise, always true
		else 
			AR(2) = UINT32_C(1);
	}
	
	// Argument 2 is negative
	else if (BN.u32 & UINT32_C(0x80000000))
	{
		// Always false (since 1st argument negativity already set)
		AR(2) = UINT32_C(0);
	}
	
	// Neither are negative
	else
	{
		if (AR(1) < BN.u32)
			AR(2) = UINT32_C(1);
		else
			AR(2) = UINT32_C(0);
	}
	
	ADVPC;
	break;

case 11:	PRINTOP(("sltiu %s, %s, %hu\n", l_RegNames[A(2)], l_RegNames[A(1)], A(3)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (AR(1) < BN.u32)
		AR(2) = UINT32_C(1);
	else
		AR(2) = UINT32_C(0);
	ADVPC;
	break;

case 12:	PRINTOP(("andi %s, %s, %hu\n", l_RegNames[A(2)], l_RegNames[A(1)], A(3)));
	AR(2) = AR(1) & A(3);
	ADVPC;
	break;

case 13:	PRINTOP(("ori %s, %s, %hu\n", l_RegNames[A(2)], l_RegNames[A(1)], A(3)));
	AR(2) = AR(1) | A(3);
	ADVPC;
	break;

case 14:	PRINTOP(("xori %s, %s, %hu\n", l_RegNames[A(2)], l_RegNames[A(1)], A(3)));
	AR(2) = AR(1) ^ A(3);
	ADVPC;
	break;

case 15:	PRINTOP(("lui %s, %hu\n", l_RegNames[A(2)], A(3)));
	AR(2) = A(3) << UINT32_C(16);
	ADVPC;
	break;

case 32:	PRINTOP(("lb %s, %hhu(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + BN.u32, 1);
	if (BN.u32 & UINT32_C(0x80))
		BN.u32 |= UINT32_C(0xFFFFFF00);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 33:	PRINTOP(("lh %s, %hi(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + BN.u32, 2);
	if (BN.u32 & UINT32_C(0x8000))
		BN.u32 |= UINT32_C(0xFFFF0000);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 35:	PRINTOP(("lw %s, %hi(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	AR(2) = MIPS_ReadMem(a_VM, AR(1) + BN.u32, 4);
	ADVPC;
	break;

case 36:	PRINTOP(("lbu %s, %hhu(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	AR(2) = MIPS_ReadMem(a_VM, AR(1) + BN.u32, 1);
	ADVPC;
	break;

case 37:	PRINTOP(("lhu %s, %hi(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	AR(2) = MIPS_ReadMem(a_VM, AR(1) + BN.u32, 2);
	ADVPC;
	break;

case 40:	PRINTOP(("sb %s, %hi(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	MIPS_WriteMem(a_VM, AR(1) + BN.u32, 1, (AR(2) & UINT32_C(0xFF)));
	ADVPC;
	break;
	
case 41:	PRINTOP(("sh %s, %hi(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	MIPS_WriteMem(a_VM, AR(1) + BN.u32, 2, (AR(2) & UINT32_C(0xFFFF)));
	ADVPC;
	break;
	
case 43:	PRINTOP(("sw %s, %hi(%s)\n", l_RegNames[A(2)], A(3), l_RegNames[A(1)]));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = A(3) | UINT32_C(0xFFFF0000);
	else
		BN.u32 = A(3);
	MIPS_WriteMem(a_VM, AR(1) + BN.u32, 4, AR(2));
	ADVPC;
	break;

BEGINARITH

case 0:		PRINTOP(("sll %s, %s, %hu\n", l_RegNames[A(3)], l_RegNames[A(2)], A(4)));
	AR(3) = AR(2) << A(4);
	ADVPC;
	break;

case 2:		PRINTOP(("srl %s, %s, %hu\n", l_RegNames[A(3)], l_RegNames[A(2)], A(4)));
	AR(3) = AR(2) >> A(4);
	ADVPC;
	break;

case 3:		PRINTOP(("sra %s, %s, %hu\n", l_RegNames[A(3)], l_RegNames[A(2)], A(4)));
	if (AR(2) & UINT32_C(0x80000000))
	{
		if (A(4) >= UINT32_C(32))
			AR(3) = UINT32_C(0xFFFFFFFF);
		else
		{
			AR(2) = AR(3);
			for (j = 0; j < A(4); j++)
			{
				AR(3) >>= 1;
				AR(3) |= UINT32_C(0x800000000);
			}
		}
	}
	
	else
		AR(3) = AR(2) >> A(4);
	
	ADVPC;
	break;

case 4:		PRINTOP(("sllv %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(2)], l_RegNames[A(1)]));
	AR(3) = AR(2) << AR(1);
	ADVPC;
	break;

case 6:		PRINTOP(("srlv %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(2)], l_RegNames[A(1)]));
	AR(3) = AR(2) >> AR(1);
	ADVPC;
	break;
	
case 7:		PRINTOP(("srav %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(2)], l_RegNames[A(1)]));
	if (AR(2) & UINT32_C(0x80000000))
	{
		if (AR(1) >= UINT32_C(32))
			AR(3) = UINT32_C(0xFFFFFFFF);
		else
		{
			AR(2) = AR(3);
			for (j = 0; j < AR(1); j++)
			{
				AR(3) >>= 1;
				AR(3) |= UINT32_C(0x800000000);
			}
		}
	}
	
	else
		AR(3) = AR(2) >> AR(1);
	
	ADVPC;
	break;

case 8:		PRINTOP(("jr %s\n", l_RegNames[A(1)]));
	JDS = AR(1);
	JDSA = 1;
	ADVPC;
	break;

case 9:		PRINTOP(("jalr %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)]));
	AR(3) = PC + UINT32_C(8);
	JDS = AR(1);
	JDSA = 1;
	ADVPC;
	break;

case 12:	PRINTOP(("syscall\n"));
	if (a_VM->SysCall)
		if (!a_VM->SysCall(a_VM, a_VM->DataP))
			i = a_Count;	// Break from loop
	ADVPC;
	break;

case 16:	PRINTOP(("mfhi %s\n", l_RegNames[A(3)]));
	AR(3) = HI;
	ADVPC;
	break;

case 17:	PRINTOP(("mthi %s\n", l_RegNames[A(1)]));
	HI = AR(1);
	ADVPC;
	break;

case 18:	PRINTOP(("mflo %s\n", l_RegNames[A(3)]));
	AR(3) = LO;
	ADVPC;
	break;

case 19:	PRINTOP(("mtlo %s\n", l_RegNames[A(1)]));
	LO = AR(1);
	ADVPC;
	break;
	
case 24:	PRINTOP(("mult %s, %s\n", l_RegNames[A(1)], l_RegNames[A(2)]));
	BN.i = ((int64_t)AR(1)) * ((int64_t)AR(2));
	HI = (BN.u >> UINT64_C(32)) & UINT64_C(0xFFFFFFFF);
	LO = BN.u & UINT64_C(0xFFFFFFFF);
	ADVPC;
	break;
	
case 25:	PRINTOP(("multu %s, %s\n", l_RegNames[A(1)], l_RegNames[A(2)]));
	BN.u = ((uint64_t)AR(1)) * ((uint64_t)AR(2));
	HI = (BN.u >> UINT64_C(32)) & UINT64_C(0xFFFFFFFF);
	LO = BN.u & UINT64_C(0xFFFFFFFF);
	ADVPC;
	break;

case 26:	PRINTOP(("div %s, %s\n", l_RegNames[A(1)], l_RegNames[A(2)]));
	if (AR(2) != UINT32_C(0))
	{
		LO = ((int32_t)AR(1)) / ((int32_t)AR(2));
		HI = ((int32_t)AR(1)) % ((int32_t)AR(2));
	}
	ADVPC;
	break;

case 27:	PRINTOP(("divu %s, %s\n", l_RegNames[A(1)], l_RegNames[A(2)]));
	if (AR(2) != UINT32_C(0))
	{
		LO = AR(1) / AR(2);
		HI = AR(1) % AR(2);
	}
	ADVPC;
	break;

	// add == addu, Since I do not support exceptions
case 32:	PRINTOP(("add %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	AR(3) = AR(1) + AR(2);
	ADVPC;
	break;

case 33:	PRINTOP(("addu %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	AR(3) = AR(1) + AR(2);
	ADVPC;
	break;

	// sub == subu, Since I do not support exceptions
case 34:	PRINTOP(("sub %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	BN.u32 = ((~AR(2)) + UINT32_C(1));
	AR(3) = AR(1) + BN.u32;
	ADVPC;
	break;

case 35:	PRINTOP(("subu %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	BN.u32 = ((~AR(2)) + UINT32_C(1));
	AR(3) = AR(1) + BN.u32;
	ADVPC;
	break;

case 36:	PRINTOP(("and %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	AR(3) = AR(1) & AR(2);
	ADVPC;
	break;

case 37:	PRINTOP(("or %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	AR(3) = AR(1) | AR(2);
	ADVPC;
	break;

case 38:	PRINTOP(("xor %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	AR(3) = AR(1) ^ AR(2);
	ADVPC;
	break;

case 39:	PRINTOP(("nor %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	AR(3) = ~(AR(1) | AR(2));
	ADVPC;
	break;

case 42:	PRINTOP(("slt %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	// Argument 1 is negative
	if (AR(1) & UINT32_C(0x80000000))
	{
		// If argument two is negative, if 1 is greater
		if (AR(2) & UINT32_C(0x80000000))
		{
			if (AR(1) > AR(2))
				AR(3) = UINT32_C(1);
			else
				AR(3) = UINT32_C(0);
		}
		
		// Otherwise, always true
		else 
			AR(3) = UINT32_C(1);
	}
	
	// Argument 2 is negative
	else if (AR(2) & UINT32_C(0x80000000))
	{
		// Always false (since 1st argument negativity already set)
		AR(3) = UINT32_C(0);
	}
	
	// Neither are negative
	else
	{
		if (AR(1) < AR(2))
			AR(3) = UINT32_C(1);
		else
			AR(3) = UINT32_C(0);
	}
	
	ADVPC;
	break;

case 43:	PRINTOP(("sltu %s, %s, %s\n", l_RegNames[A(3)], l_RegNames[A(1)], l_RegNames[A(2)]));
	if (AR(1) < AR(2))
		AR(3) = UINT32_C(1);
	else
		AR(3) = UINT32_C(0);
	ADVPC;
	break;

default:	PRINTOP(("unka%02x\n", Am[5]));
	ADVPC;
	break;

ENDARITH

default:	PRINTOP(("unk%02x\n", Am[0]));
	ADVPC;
	break;

#undef PC
#undef R
/*---------------------------------------------------------------------------*/
		}
		
		// Always reset register zero to zero
		a_VM->CPU.r[0] = 0;
		
		// Print changes in CPU registers
#if defined(_DEBUG)
		if (a_PrintOp)
		{
			for (x = 0; x < 32; x++)
				if (OldCPU.r[x] != a_VM->CPU.r[x])
					CONL_PrintF("%s = (0x%08x -> 0x%08x)\n", l_RegNames[x], OldCPU.r[x], a_VM->CPU.r[x]);
					
			if (OldCPU.hi != OldCPU.hi)
				CONL_PrintF("$hi = (0x%08x -> 0x%08x)\n", OldCPU.hi, a_VM->CPU.hi);
			if (OldCPU.lo != OldCPU.lo)
				CONL_PrintF("$lo = (0x%08x -> 0x%08x)\n", OldCPU.lo, a_VM->CPU.lo);
		}
#endif
	}
	
	/* No Exceptions met */
	return true;
}

