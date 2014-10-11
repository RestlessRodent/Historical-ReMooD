// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: This is a MIPS execution tester, which determines whether or

/***************
*** INCLUDES ***
***************/

#if !defined(__linux__)
	#include "bot_lib.h"
#else
	#include <stdint.h>
	#include <stdio.h>
	
	#define BSleep(x) exit(0)
	
	#define GeneralChat(x) puts(x)
#endif

/****************
*** FUNCTIONS ***
****************/

/* main() -- Bot main entry */
void main(void)
{
#define __MEM asm __volatile__ (".set noat"); asm __volatile__("" : : : "memory")

#define VA "$s5"
#define VB "$s6"
#define VC "$s7"
	
#define FAILPASS(x) \
	GeneralChat("FAIL: "x);\
else\
	GeneralChat("PASS: "x);
	
#define PASSFAIL(x) \
	GeneralChat("PASS: "x);\
else\
	GeneralChat("FAIL: "x);

	volatile register union
	{
		signed i;
		unsigned u;
	} var1 asm("s5");
	
	volatile register union
	{
		signed i;
		unsigned u;
	} var2 asm("s6");
	
	volatile register union
	{
		signed i;
		unsigned u;
	} var3 asm("s7");
	
	uint32_t LVar[3] = {UINT32_C(0x1337D00D), UINT32_C(0x80BADE42), UINT32_C(0xCAFEBABE)};
	
	/* Initial Barrier */
	__MEM;
	
	/*** STANDARD OPS ***/
	
	/* ADD/ADDU */
	var1.u = UINT32_C(0x12345678);
	var2.u = UINT32_C(0x87654321);
	
	asm __volatile__("addu "VC", "VA", "VB"");
	
	if (var3.u != UINT32_C(0x99999999))
		FAILPASS("ADD/ADDU");
	
	__MEM;
	
	/* ADDI/ADDIU */
	var1.u = UINT32_C(0x12345678);
	
	asm __volatile__("addiu "VC", "VA", 0x8642");
	
	if (var3.u != UINT32_C(0x1233DCBA))
		FAILPASS("ADDI/ADDIU");
	
	__MEM;
	
	/* AND */
	var1.u = UINT32_C(0xDEADBEEF);
	var2.u = UINT32_C(0xCAFEBABE);
	
	asm __volatile__("and "VC", "VA", "VB"");
	
	if (var3.u != UINT32_C(0xCAACBAAE))
		FAILPASS("AND");
	
	__MEM;
	
	/* ANDI */
	var1.u = UINT32_C(0xCAFEBABE);
	
	asm __volatile__("andi "VC", "VA", 0x1337");
	
	if (var3.u != UINT32_C(0x00001236))
		FAILPASS("ANDI");
	
	__MEM;
	
	/* XOR */
	var1.u = UINT32_C(0x1337F00F);
	var2.u = UINT32_C(0xDEADCAFE);
	
	asm __volatile__("xor "VC", "VA", "VB"");
	
	if (var3.u != UINT32_C(0xCD9A3AF1))
		FAILPASS("XOR");
	
	__MEM;
	
	/* XORI */
	var1.u = UINT32_C(0x8BADF00D);
	
	asm __volatile__("xori "VC", "VA", 0xBADA");
	
	if (var3.u != UINT32_C(0x8BAD4AD7))
		FAILPASS("XORI");
	
	__MEM;
	
	/* DIV */
	// TODO FIXME
	
	/* DIVU */
	var1.i = INT32_C(25);
	var2.i = INT32_C(6);
	
	asm __volatile__("divu "VA", "VB"");
	
	asm __volatile__("mfhi "VC"");
	if (var3.i != INT32_C(1))
		FAILPASS("DIV (HI)");
	
	asm __volatile__("mflo "VC"");
	if (var3.i != INT32_C(4))
		FAILPASS("DIV (LO)");
	
	__MEM;
	
	/* LB */
	var1.u = (uint32_t)&LVar[1];
	
	asm __volatile__("lb "VC", 4("VA")");
	asm __volatile__("lb "VB", 0("VA")");
	asm __volatile__("lb "VA", -4("VA")");
	
	if (var1.u != UINT32_C(0x0000000D))
		FAILPASS("LB (4)");
	
	if (var2.u != UINT32_C(0x00000042))
		FAILPASS("LB (0)");
	
	if (var3.u != UINT32_C(0xFFFFFFBE))
		FAILPASS("LB (-4)");
	
	__MEM;
		
	/* LW */
	var1.u = (uint32_t)&LVar[1];
	
	asm __volatile__("lw "VC", 4("VA")");
	asm __volatile__("lw "VB", 0("VA")");
	asm __volatile__("lw "VA", -4("VA")");
	
	if (var1.u != UINT32_C(0x1337D00D))
		FAILPASS("LW (4)");
	
	if (var2.u != UINT32_C(0x80BADE42))
		FAILPASS("LW (0)");
	
	if (var3.u != UINT32_C(0xCAFEBABE))
		FAILPASS("LW (-4)");
	
	__MEM;
	
	/* LUI */
	asm __volatile__("lui "VC", 0xCAFE");
	
	if (var3.u != UINT32_C(0xCAFE0000))
		FAILPASS("LUI");
	
	__MEM;
	
	/* SLT */
	var1.i = INT32_C(70);
	var2.i = INT32_C(99);
	
	asm __volatile__("slt "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLT 70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	var2.i = INT32_C(70);
	
	asm __volatile__("slt "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLT 99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(-70);
	var2.i = INT32_C(99);
	
	asm __volatile__("slt "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLT -70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(-99);
	var2.i = INT32_C(70);
	
	asm __volatile__("slt "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLT -99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(70);
	var2.i = INT32_C(-99);
	
	asm __volatile__("slt "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLT 70 < -99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	var2.i = INT32_C(-70);
	
	asm __volatile__("slt "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLT 99 < -70");
		
	__MEM;
	
	/* SLTU */
	var1.i = INT32_C(70);
	var2.i = INT32_C(99);
	
	asm __volatile__("sltu "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTU 70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	var2.i = INT32_C(70);
	
	asm __volatile__("sltu "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTU 99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(-70);
	var2.i = INT32_C(99);
	
	asm __volatile__("sltu "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTU -70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(-99);
	var2.i = INT32_C(70);
	
	asm __volatile__("sltu "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTU -99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(70);
	var2.i = INT32_C(-99);
	
	asm __volatile__("sltu "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTU 70 < -99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	var2.i = INT32_C(-70);
	
	asm __volatile__("sltu "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTU 99 < -70");
		
	__MEM;
	
	/* SLTI */
	var1.i = INT32_C(70);
	
	asm __volatile__("slti "VC", "VA", 99");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTI 70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	
	asm __volatile__("slti "VC", "VA", 70");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTI 99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(-70);
	
	asm __volatile__("slti "VC", "VA", 99");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTI -70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(-99);
	
	asm __volatile__("slti "VC", "VA", 70");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTI -99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(70);
	
	asm __volatile__("slti "VC", "VA", -99");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTI 70 < -99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	
	asm __volatile__("slti "VC", "VA", -70");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTI 99 < -70");
		
	__MEM;
	
	/* SLTIU */
	var1.i = INT32_C(70);
	
	asm __volatile__("sltiu "VC", "VA", -99");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTIU 70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	
	asm __volatile__("sltiu "VC", "VA", 70");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTIU 99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(-70);
	
	asm __volatile__("sltiu "VC", "VA", 99");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTIU -70 < 99");
		
	__MEM;
		
	var1.i = INT32_C(-99);
	
	asm __volatile__("sltiu "VC", "VA", 70");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SLTIU -99 < 70");
		
	__MEM;
		
	var1.i = INT32_C(70);
	
	asm __volatile__("sltiu "VC", "VA", -99");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTIU 70 < -99");
		
	__MEM;
		
	var1.i = INT32_C(99);
	
	asm __volatile__("sltiu "VC", "VA", -70");
	
	if (var3.u == UINT32_C(0x00000001))
		PASSFAIL("SLTIU 99 < -70");
		
	__MEM;
	
	/* SRA */
	var1.u = UINT32_C(0x13370000);
	
	asm __volatile__("sra "VC", "VA", 16");
	
	if (var3.u == UINT32_C(0x00001337))
		PASSFAIL("SRA 0x13370000 >>= 16");
	
	__MEM;
	
	var1.u = UINT32_C(0x13370000);
	
	asm __volatile__("sra "VC", "VA", 24");
	
	if (var3.u == UINT32_C(0x00000013))
		PASSFAIL("SRA 0x13370000 >>= 24");
	
	__MEM;
	
	var1.u = UINT32_C(0x13370000);
	
	asm __volatile__("sra "VC", "VA", 31");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SRA 0x13370000 >>= 31");
	
	__MEM;
	
	var1.u = UINT32_C(0x93370000);
	
	asm __volatile__("sra "VC", "VA", 16");
	
	if (var3.u == UINT32_C(0xFFFF9337))
		PASSFAIL("SRA 0x93370000 >>= 16");
	
	__MEM;
	
	var1.u = UINT32_C(0x93370000);
	
	asm __volatile__("sra "VC", "VA", 24");
	
	if (var3.u == UINT32_C(0xFFFFFF93))
		PASSFAIL("SRA 0x93370000 >>= 24");
	
	__MEM;
	
	var1.u = UINT32_C(0x93370000);
	
	asm __volatile__("sra "VC", "VA", 31");
	
	if (var3.u == UINT32_C(0xFFFFFFFF))
		PASSFAIL("SRA 0x93370000 >>= 31");
	
	__MEM;
	
	/* SRAV */
	var1.u = UINT32_C(0x13370000);
	var2.u = UINT32_C(16);
	
	asm __volatile__("srav "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00001337))
		PASSFAIL("SRAV 0x13370000 >>= 16");
	
	__MEM;
	
	var1.u = UINT32_C(0x13370000);
	var2.u = UINT32_C(24);
	
	asm __volatile__("srav "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000013))
		PASSFAIL("SRAV 0x13370000 >>= 24");
	
	__MEM;
	
	var1.u = UINT32_C(0x13370000);
	var2.u = UINT32_C(31);
	
	asm __volatile__("srav "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0x00000000))
		PASSFAIL("SRAV 0x13370000 >>= 31");
	
	__MEM;
	
	var1.u = UINT32_C(0x93370000);
	var2.u = UINT32_C(16);
	
	asm __volatile__("srav "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0xFFFF9337))
		PASSFAIL("SRAV 0x93370000 >>= 16");
	
	__MEM;
	
	var1.u = UINT32_C(0x93370000);
	var2.u = UINT32_C(24);
	
	asm __volatile__("srav "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0xFFFFFF93))
		PASSFAIL("SRAV 0x93370000 >>= 24");
	
	__MEM;
	
	var1.u = UINT32_C(0x93370000);
	var2.u = UINT32_C(31);
	
	asm __volatile__("srav "VC", "VA", "VB"");
	
	if (var3.u == UINT32_C(0xFFFFFFFF))
		PASSFAIL("SRAV 0x93370000 >>= 31");
	
	__MEM;
	
	/* MF SERIES */
#if 0
	var1.u = UINT32_C(0xDEADBEEF);
	var2.u = UINT32_C(0xCAFEBABE);
	
	asm __volatile__("mtlo "VA"");
	asm __volatile__("mthi "VB"");
	
	var1.u = UINT32_C(0x0BADB002);
	var2.u = UINT32_C(
	
	asm __volatile__("nop");	// mf* requires 2 instruction gap
	asm __volatile__("nop");
#endif
	
	/* Done So Loop Forever */
	__MEM;
	for (;;)
		BSleep(10000);
}

