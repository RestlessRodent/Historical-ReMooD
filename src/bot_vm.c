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
// DESCRIPTION: Virtual Machine Helpers for bots

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "bot.h"
#include "sn.h"
#include "d_netcmd.h"
#include "bot_priv.h"
#include "console.h"
#include "g_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "m_argv.h"
#include "d_player.h"
#include "bot_lib.h"
#include "r_defs.h"
#include "r_state.h"
#include "p_local.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "m_random.h"

/****************
*** FUNCTIONS ***
****************/

/* BOT_VMFDataStructs() -- Data Structure Handling */
static uint32_t BOT_VMFDataStructs(MIPS_VM_t* const a_VM, MIPS_Map_t* const a_Map, const uint_fast32_t a_BaseAddr)
{
	uint32_t Index;
	uint32_t ABase, SBase;
	union
	{
		mobj_t* mo;
		sector_t* Sect;
		mapthing_t* MT;
		node_t* Node;
		subsector_t* Sub;
		line_t* Line;
		side_t* Side;
		seg_t* Seg;
		msecnode_t* MSec;
		ffloor_t* FF;
		vertex_t* V;
	} p;
	
	/* Mobjs */
	if (a_BaseAddr >= MOBJECTBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MOBJECTBASEADDR);
		Index = ABase / MOBJECTSIZE;
		SBase = ABase % MOBJECTSIZE;
		
		return 0;
	}
	
	/* Sectors */
	else if (a_BaseAddr >= MSECTORBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MSECTORBASEADDR);
		Index = ABase / MSECTORSIZE;
		SBase = ABase % MSECTORSIZE;
		
		if (Index < numsectors)
			p.Sect = &sectors[Index];
		else
			return 0;
	}
	
	/* Map Things */
	else if (a_BaseAddr >= MTHINGBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MTHINGBASEADDR);
		Index = ABase / MTHINGSIZE;
		SBase = ABase % MTHINGSIZE;
		
		if (Index < nummapthings)
			p.MT = &mapthings[Index];
		else
			return 0;
	}
	
	/* Nodes */
	else if (a_BaseAddr >= MNODEBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MNODEBASEADDR);
		Index = ABase / MNODESIZE;
		SBase = ABase % MNODESIZE;
		
		if (Index < numnodes)
			p.Node = &nodes[Index];
		else
			return 0;
	}
	
	/* Sub Sector */
	else if (a_BaseAddr >= MSUBSBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MSUBSBASEADDR);
		Index = ABase / MSUBSSIZE;
		SBase = ABase % MSUBSSIZE;
		
		if (Index < numsubsectors)
			p.Sub = &subsectors[Index];
		else
			return 0;
	}
	
	/* LineDef */
	else if (a_BaseAddr >= MLINEBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MLINEBASEADDR);
		Index = ABase / MLINESIZE;
		SBase = ABase % MLINESIZE;
		
		if (Index < numlines)
			p.Line = &lines[Index];
		else
			return 0;
	}
	
	/* SideDef */
	else if (a_BaseAddr >= MSIDEBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MSIDEBASEADDR);
		Index = ABase / MSIDESIZE;
		SBase = ABase % MSIDESIZE;
		
		if (Index < numsides)
			p.Side = &sides[Index];
		else
			return 0;
	}
	
	/* Segs */
	else if (a_BaseAddr >= MSEGBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MSEGBASEADDR);
		Index = ABase / MSEGSIZE;
		SBase = ABase % MSEGSIZE;
		
		if (Index < numsegs)
			p.Seg = &segs[Index];
		else
			return 0;
	}
	
	/* Sector Nodes */
	else if (a_BaseAddr >= MSECNODEBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MSECNODEBASEADDR);
		Index = ABase / MSECNODESIZE;
		SBase = ABase % MSECNODESIZE;
		
		if (Index < g_NumMSecNodes)
			p.MSec = g_MSecNodes[Index];
		else
			return 0;
		
		// Illegal SecNode?
		if (!p.MSec)
			return 0;
	}
	
	/* Fake Floors */
	else if (a_BaseAddr >= MFFBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MFFBASEADDR);
		Index = ABase / MFFSIZE;
		SBase = ABase % MFFSIZE;
		
		if (Index < g_NumPFakeFloors)
			p.FF = g_PFakeFloors[Index];
		else
			return 0;
		
		// Illegal Floor?
		if (!p.FF)
			return 0;
	}
	
	/* Vertexes */
	else if (a_BaseAddr >= MVERTEXBASEADDR)
	{
		// Obtain Index
		ABase = (a_BaseAddr - MVERTEXBASEADDR);
		Index = ABase / MVERTEXSIZE;
		SBase = ABase % MVERTEXSIZE;
		
		if (Index < numvertexes)
			p.V = &vertexes[Index];
		else
			return 0;
		
		// Based on struct addresss
		switch (SBase)
		{
			case 0: return p.V->x;
			case 4: return p.V->y;
		}
	}
	
	/* Unknown */
	return 0;
}

/* BOT_SysCall() -- Handles system calls */
static bool_t BOT_SysCall(MIPS_VM_t* const a_VM, void* const a_Data)
{
#define BUFSIZE 256
	char Buf[BUFSIZE];
	int i;
	BOT_t* Bot = a_Data;
	uint32_t ko = a_VM->CPU.r[MIPS_K0];
	uint32_t kl = a_VM->CPU.r[MIPS_K1];
	
	fixed_t fa, fb, fc, fd;
	
	/* Based on syscall performed */
	switch (ko)
	{
			// Sleep(len)
		case BOTSYSCALL_BSLEEP:
			Bot->SleepCount = MIPS_ReadMemX(a_VM, kl, 4);
			return false;	// Break execution for a bit
		
			// FMul(a, b) -> c
		case BOTSYSCALL_FMUL:
			a_VM->CPU.r[MIPS_K1] = FixedMul(MIPS_ReadMemX(a_VM, kl, 4), MIPS_ReadMemX(a_VM, kl + 4, 4));
			break;
			
			// DistTo(a, b) -> c
		case BOTSYSCALL_DISTTO:
			fa = Bot->BInfo.x;
			fb = Bot->BInfo.y;
			fc = MIPS_ReadMemX(a_VM, kl, 4);
			fd = MIPS_ReadMemX(a_VM, kl + 4, 4);
			
			a_VM->CPU.r[MIPS_K1] = P_AproxDistance(fc - fa, fd - fb);
			break;
			
			// SightTo(a, b) -> true/false
		case BOTSYSCALL_SIGHTTO:
			fa = Bot->BInfo.x;
			fb = Bot->BInfo.y;
			fc = MIPS_ReadMemX(a_VM, kl, 4);
			fd = MIPS_ReadMemX(a_VM, kl + 4, 4);
			
			a_VM->CPU.r[MIPS_K1] = P_CheckSightLine(fa, fb, fc, fd);
			break;
			
			// Chat Message (A bit slow)
		case BOTSYSCALL_CHAT:
			// Read chat buffer
			memset(Buf, 0, sizeof(Buf));
			for (i = 0; i < BUFSIZE - 1; i++)
			{
				Buf[i] = MIPS_ReadMemX(a_VM, kl + i, 1);
				
				// No more characters
				if (!Buf[i])
					break;
				
				// Do not permit below non-ASCII characters
				if (Buf[i] < ' ' || Buf[i] > 0x7F)
					Buf[i] = '?';
			}
			
			// Send chat to SN system
			if (Buf[0])
				SN_SendChat(Bot->Port, false, Buf);
			break;
		
			// Returns a random number
		case BOTSYSCALL_RANDLTZGT:
			fa = M_Random() & 3;
			
			a_VM->CPU.r[MIPS_K1] = !!(fa & 1);
			
			if (fa & 2)
				a_VM->CPU.r[MIPS_K1] = (~a_VM->CPU.r[MIPS_K1]) + 1;
			
			// Unknown SysCall
		default:
			break;
	}
	
#define BOTSYSCALL_DISTTO		UINT32_C(0x00000003)
#define BOTSYSCALL_SIGHTTO		UINT32_C(0x00000004)
	
	/* Always continue */
	return true;
#undef BUFSIZE
}

/* BOT_RegisterVMJunk() -- Registers virtual machine hanlders */
void BOT_RegisterVMJunk(BOT_t* const a_Bot)
{
	MIPS_VM_t* VM;
	
	/* Check */
	if (!a_Bot)
		return;
	
	/* Get virtual machine pointer */
	VM = &a_Bot->VM;
	
	/* Handle System Calls */
	VM->DataP = a_Bot;
	VM->SysCall = BOT_SysCall;
	
	/* Map Data Structure Handler */
	MIPS_VMAddMapFunc(VM, BOT_VMFDataStructs, NULL, MDSBASEADDR, MDSENDADDR - MDSBASEADDR, MIPS_MFR);
	
	/* Map Static R (possibly W) structures */
	// Information on the bot
	MIPS_VMAddMap(VM, &a_Bot->BInfo, BBINFOBASEADDR, sizeof(a_Bot->BInfo), MIPS_MFR | MIPS_MFW);
	
	// Real Time
	MIPS_VMAddMap(VM, &g_BotRT, BBREALBASEADDR, sizeof(g_BotRT), MIPS_MFR);
}

