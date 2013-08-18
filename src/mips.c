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
			return LittleSwapUInt32(((uint32_t*)Map->RealMem)[BaseOff >> 2]);
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
			((uint16_t*)Map->RealMem)[BaseOff >> 1] = LittleSwapUInt16(a_Val);
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
		// 26 bits of absolute stuff
		a_Out[3] = (a_Op) & UINT32_C(0x3FFFFFF);			// i
		
		a_Out[1] = 0;
		a_Out[2] = 0;
		a_Out[4] = 0;
		a_Out[5] = 0;
		return;
	}
	
	/* Otherwise, immediate mode */
	// 33222222 22221111 111111
	// 10987654 32109876 54321098 76543210
	// ooooooss sssttttt iiiiiiii iiiiiiii
	a_Out[1] = (a_Op >> UINT32_C(21)) & UINT32_C(31);		// s
	a_Out[2] = (a_Op >> UINT32_C(16)) & UINT32_C(31);		// t
	a_Out[3] = (a_Op) & UINT32_C(0xFFFF);					// i
	
	a_Out[4] = 0;
	a_Out[5] = 0;
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
		// Read memory at PC
		Op = MIPS_ReadMem(a_VM, a_VM->CPU.pc, 4);
		
		// Swap operator on big endian
#if defined(__REMOOD_BIG_ENDIAN)
		Op = LittleSwapUInt32(op);
#endif
		
		// No operation? Do not bother entering switch loop
		if (!Op)
		{
			// Increase nop count
			NopCount++;
			
			// Increase PC
			a_VM->CPU.pc += UINT32_C(4);
			
			// 3 Nops trigger sleep, otherwise reloop
			if (NopCount == 3)
				break;
			continue;
		}
		
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
#define FOUR UINT32_C(4);
#define ADVPC PC += FOUR

case 2:		PRINTOP(("j 0x%08x\n", A(3) << UINT32_C(2)));
	PC = A(3) << UINT32_C(2);
	break;

case 3:		PRINTOP(("jal 0x%08x\n", A(3) << UINT32_C(2)));
	R(31) = PC + UINT32_C(8);
	PC = A(3) << UINT32_C(2);
	break;

case 4:		PRINTOP(("beq $%u, $%u, %u\n", A(1), A(2), A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (AR(1) == AR(2))
		PC += BN.u32 << UINT32_C(2);
	else
		ADVPC;
	break;

case 5:		PRINTOP(("bne $%u, $%u, %u\n", A(1), A(2), A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (AR(1) != AR(2))
		PC += BN.u32 << UINT32_C(2);
	else
		ADVPC;
	break;

case 6:		PRINTOP(("blez $%u, %i\n", A(1), A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (((int32_t)AR(1)) <= INT32_C(0))
		PC += BN.u32 << UINT32_C(2);
	else
		ADVPC;
	break;

case 7:		PRINTOP(("bgtz $%u, %i\n", A(1), A(3) << UINT32_C(2)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	
	if (((int32_t)AR(1)) > INT32_C(0))
		PC += BN.u32 << UINT32_C(2);
	else
		ADVPC;
	break;

case 8:		PRINTOP(("addi $%u, $%u, %i\n", A(2), A(1), A(3)));
	if (A(3) & UINT32_C(0x8000))
		BN.i32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.i32 = A(3);
	AR(2) = ((int32_t)AR(1)) + BN.i32;
	ADVPC;
	break;

case 9:		PRINTOP(("addiu $%u, $%u, %u\n", A(2), A(1), A(3)));
	if (A(3) & UINT32_C(0x8000))
		BN.u32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.u32 = A(3);
	AR(2) = AR(1) + A(3);
	ADVPC;
	break;

case 10:	PRINTOP(("slti $%u, $%u, %i\n", A(2), A(1), A(3)));
	if (A(3) & UINT32_C(0x8000))
		BN.i32 = UINT32_C(0xFFFF0000) | A(3);
	else
		BN.i32 = A(3);
	
	if (((int32_t)AR(1)) < BN.i32)
		AR(2) = UINT32_C(1);
	else
		AR(2) = UINT32_C(0);
	ADVPC;
	break;

case 11:	PRINTOP(("sltiu $%u, $%u, %u\n", A(2), A(1), A(3)));
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

case 12:	PRINTOP(("andi $%u, $%u, %u\n", A(2), A(1), A(3)));
	AR(2) = AR(1) & A(3);
	ADVPC;
	break;

case 13:	PRINTOP(("ori $%u, $%u, %u\n", A(2), A(1), A(3)));
	AR(2) = AR(1) | A(3);
	ADVPC;
	break;

case 14:	PRINTOP(("xori $%u, $%u, %u\n", A(2), A(1), A(3)));
	AR(2) = AR(1) ^ A(3);
	ADVPC;
	break;

case 15:	PRINTOP(("lui $%u, %u\n", A(2), A(3)));
	AR(2) = A(3) << UINT32_C(16);
	ADVPC;
	break;

case 24:	PRINTOP(("llo $%u, %u\n", A(2), A(3)));
	AR(2) = (AR(2) & UINT32_C(0x0000FFFF)) | A(3);
	ADVPC;
	break;

case 25:	PRINTOP(("lhi $%u, %u\n", A(2), A(3)));
	AR(2) = (AR(2) & UINT32_C(0xFFFF0000)) | (A(3) << UINT32_C(16));
	ADVPC;
	break;

case 32:	PRINTOP(("lb $%u, %08x($%u)\n", A(2), A(3), A(1)));
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + A(3), 1);
	if (BN.u32 & UINT32_C(0x80))
		BN.u32 |= UINT32_C(0xFFFFFF00);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 33:	PRINTOP(("lh $%u, %08x($%u)\n", A(2), A(3), A(1)));
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + A(3), 2);
	if (BN.u32 & UINT32_C(0x8000))
		BN.u32 |= UINT32_C(0xFFFF0000);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 35:	PRINTOP(("lw $%u, %4x($%u)\n", A(2), A(3), A(1)));
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + A(3), 4);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 36:	PRINTOP(("lbu $%u, %4x($%u)\n", A(2), A(3), A(1)));
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + A(3), 1);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 37:	PRINTOP(("lhu $%u, %4x($%u)\n", A(2), A(3), A(1)));
	BN.u32 = MIPS_ReadMem(a_VM, AR(1) + A(3), 2);
	AR(2) = BN.u32;
	ADVPC;
	break;

case 40:	PRINTOP(("sb $%u, %4x($%u)\n", A(2), A(3), A(1)));
	MIPS_WriteMem(a_VM, AR(1) + A(3), 1, AR(2));
	ADVPC;
	break;
	
case 41:	PRINTOP(("sh $%u, %4x($%u)\n", A(2), A(3), A(1)));
	MIPS_WriteMem(a_VM, AR(1) + A(3), 2, AR(2));
	ADVPC;
	break;
	
case 43:	PRINTOP(("sw $%u, %4x($%u)\n", A(2), A(3), A(1)));
	MIPS_WriteMem(a_VM, AR(1) + A(3), 4, AR(2));
	ADVPC;
	break;

BEGINARITH

case 0:		PRINTOP(("sll $%u, $%u, %u\n", A(3), A(2), A(4)));
	AR(3) = AR(2) << A(4);
	ADVPC;
	break;

case 2:		PRINTOP(("srl $%u, $%u, %u\n", A(3), A(2), A(4)));
	AR(3) = AR(2) >> A(4);
	ADVPC;
	break;

case 3:		PRINTOP(("sra $%u, $%u, %u\n", A(3), A(2), A(4)));
	AR(3) = ((int32_t)AR(2)) >> ((int32_t)A(4));
	ADVPC;
	break;

case 4:		PRINTOP(("sllv $%u, $%u, $%u\n", A(3), A(2), A(1)));
	AR(3) = AR(2) << AR(1);
	ADVPC;
	break;

case 6:		PRINTOP(("srlv $%u, $%u, $%u\n", A(3), A(2), A(1)));
	AR(3) = AR(2) >> AR(1);
	ADVPC;
	break;
	
case 7:		PRINTOP(("srav $%u, $%u, $%u\n", A(3), A(2), A(1)));
	AR(3) = ((int32_t)AR(2)) >> ((int32_t)AR(1));
	ADVPC;
	break;

case 8:		PRINTOP(("jr $%u\n", A(1)));
	PC = AR(1);
	break;

case 9:		PRINTOP(("jalr $%u\n", A(1)));
	R(31) = PC + UINT32_C(8);
	PC = AR(1) << UINT32_C(2);
	break;

case 16:	PRINTOP(("mfhi $%u\n", A(3)));
	AR(3) = HI;
	ADVPC;
	break;

case 17:	PRINTOP(("mthi $%u\n", A(1)));
	HI = AR(1);
	ADVPC;
	break;

case 18:	PRINTOP(("mflo $%u\n", A(3)));
	AR(3) = LO;
	ADVPC;
	break;

case 19:	PRINTOP(("mtlo $%u\n", A(1)));
	LO = AR(1);
	ADVPC;
	break;
	
case 24:	PRINTOP(("mult $%u, $%u\n", A(1), A(2)));
	BN.i = ((int64_t)AR(1)) * ((int64_t)AR(2));
	HI = (BN.u >> UINT64_C(32)) & UINT64_C(0xFFFFFFFF);
	LO = BN.u & UINT64_C(0xFFFFFFFF);
	ADVPC;
	break;
	
case 25:	PRINTOP(("multu $%u, $%u\n", A(1), A(2)));
	BN.u = ((uint64_t)AR(1)) * ((uint64_t)AR(2));
	HI = (BN.u >> UINT64_C(32)) & UINT64_C(0xFFFFFFFF);
	LO = BN.u & UINT64_C(0xFFFFFFFF);
	ADVPC;
	break;

case 26:	PRINTOP(("div $%u, $%u\n", A(1), A(2)));
	if (AR(2) != UINT32_C(0))
	{
		LO = ((int32_t)AR(1)) / ((int32_t)AR(2));
		HI = ((int32_t)AR(1)) % ((int32_t)AR(2));
	}
	ADVPC;
	break;

case 27:	PRINTOP(("divu $%u, $%u\n", A(1), A(2)));
	if (AR(2) != UINT32_C(0))
	{
		LO = AR(1) / AR(2);
		HI = AR(1) % AR(2);
	}
	ADVPC;
	break;

case 32:	PRINTOP(("add $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = ((int32_t)AR(1)) + ((int32_t)AR(2));
	ADVPC;
	break;

case 33:	PRINTOP(("addu $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = AR(1) + AR(2);
	ADVPC;
	break;

case 34:	PRINTOP(("sub $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = ((int32_t)AR(1)) - ((int32_t)AR(2));
	ADVPC;
	break;

case 35:	PRINTOP(("subu $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = AR(1) - AR(2);
	ADVPC;
	break;

case 36:	PRINTOP(("and $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = AR(1) & AR(2);
	ADVPC;
	break;

case 37:	PRINTOP(("or $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = AR(1) | AR(2);
	ADVPC;
	break;

case 38:	PRINTOP(("xor $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = AR(1) ^ AR(2);
	ADVPC;
	break;

case 39:	PRINTOP(("nor $%u, $%u, $%u\n", A(3), A(1), A(2)));
	AR(3) = ~(AR(1) | AR(2));
	ADVPC;
	break;

case 42:	PRINTOP(("slt $%u, $%u, $%u\n", A(3), A(1), A(2)));
	if (((int32_t)AR(1)) < ((int32_t)AR(2)))
		AR(3) = UINT32_C(1);
	else
		AR(3) = UINT32_C(0);
	ADVPC;
	break;

case 43:	PRINTOP(("sltu $%u, $%u, $%u\n", A(3), A(1), A(2)));
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
			for (x = 0; x < 32; x++)
				if (OldCPU.r[x] != a_VM->CPU.r[x])
					CONL_PrintF("r%i = (0x%08x -> 0x%08x)\n", x, OldCPU.r[x], a_VM->CPU.r[x]);
#endif
	}
	
	/* No Exceptions met */
	return true;
}

